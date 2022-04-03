/**
 * @file tai_gokart_interface.cpp
 * @author Haoru Xue (haoru.xue@autoware.org)
 * @brief
 * @version 0.1
 * @date 2022-02-06
 *
 * @copyright Copyright 2022 Triton AI
 *
 */

#include <string>
#include <memory>

#include "tai_gokart_controller/tai_gokart_interface.hpp"
#include "tai_gokart_packet/version.hpp"

namespace tritonai
{
namespace gkc
{
GkcInterface::GkcInterface(const ConfigList & configs)
: factory_(std::make_unique<GkcPacketFactory>(this, GkcPacketUtils::debug_cout))
{
  // Find and initialize a comm interface based on config
  std::string comm_name = static_cast<std::string>(configs.at("comm_type"));
  comm_ = comm_lookup_.at(comm_name)(this);
  if (!comm_) {
    throw std::runtime_error("Cannot find comm interface with name \"" + comm_name + ".\"");
  }

  // Initialize the communication
  if (comm_->configure(configs) && comm_->open() && send_handshake()) {
  } else {
    throw std::runtime_error("Communication to the MCU cannot be established.");
  }

  static constexpr uint32_t MAX_HANDSHAKE_WAIT_MS = 3000;
  const std::chrono::milliseconds max_wait_ms(MAX_HANDSHAKE_WAIT_MS);
  std::this_thread::sleep_for(max_wait_ms);
  if (!handshake_good_) {
    throw std::runtime_error("GKC handshake timeout.");
  }

  // Send firmware version inqury
  send_firmware_version_request();

  // Start streaming heartbeats
  auto log = LogPacket();
  log.level = LogPacket::Severity::INFO;
  log.what = "Start streaming heartbeats.";
  logs_.emplace(log);
  heartbeat_thread =
    std::unique_ptr<std::thread>(new std::thread(&GkcInterface::stream_heartbeats, this));
}

GkcInterface::~GkcInterface()
{
  if (comm_) {
    comm_->close();
  }
  if (heartbeat_thread && heartbeat_thread->joinable()) {
    heartbeat_thread->join();
  }
}

bool GkcInterface::send_control(const ControlGkcPacket & control_packet)
{
  if (!comm_ || !comm_->is_open()) {
    return false;
  }
  return static_cast<bool>(comm_->send(*factory_->Send(control_packet)));
}

bool GkcInterface::initialize(const ConfigGkcPacket & config_packet, const uint32_t & timeout_ms)
{
  if (current_state_ != GkcLifecycle::Uninitialized) {
    auto log = LogPacket();
    log.level = LogPacket::Severity::WARNING;
    log.what = "GKC can only be initialized in uninitialized state. Current state is " +
      std::to_string(current_state_) + ".";
    logs_.emplace(log);
    return false;
  }
  if (!comm_ || !comm_->is_open()) {
    return false;
  }
  initialized_ = true;
  auto sent = static_cast<bool>(comm_->send(*factory_->Send(config_packet)));
  if (!sent) {
    return false;
  }

  // Wait for initialization on the MCU
  std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));

  // Check state code to determine if initialization was successful
  initialized_ = current_state_ == GkcLifecycle::Initializing ||
    current_state_ == GkcLifecycle::Inactive;
  return initialized_;
}

bool GkcInterface::activate(const uint32_t & timeout_ms)
{
  if (current_state_ != GkcLifecycle::Inactive) {
    auto log = LogPacket();
    log.level = LogPacket::Severity::WARNING;
    log.what = "GKC can only be initialized in inactive state. Current state is " +
      std::to_string(current_state_) + ".";
    logs_.emplace(log);
    return false;
  }
  return try_change_state(GkcLifecycle::Active, timeout_ms);
}

bool GkcInterface::deactivate(const uint32_t & timeout_ms)
{
  if (current_state_ != GkcLifecycle::Active) {
    auto log = LogPacket();
    log.level = LogPacket::Severity::WARNING;
    log.what = "GKC can only be deactivate in active state. Current state is " +
      std::to_string(current_state_) + ".";
    logs_.emplace(log);
    return false;
  }
  return try_change_state(GkcLifecycle::Inactive, timeout_ms);
}

bool GkcInterface::emergency_stop(const uint32_t & timeout_ms)
{
  if (current_state_ == GkcLifecycle::Uninitialized) {
    auto log = LogPacket();
    log.level = LogPacket::Severity::WARNING;
    log.what = "GKC can not go to emergency state in uninitialized state.";
    logs_.emplace(log);
    return false;
  }
  return try_change_state(GkcLifecycle::Emergency, timeout_ms);
}

bool GkcInterface::release_emergency_stop(const uint32_t & timeout_ms)
{
  (void) timeout_ms;
  auto log = LogPacket();
  log.level = LogPacket::Severity::ERROR;
  log.what = "Releasing estop is not implemented. Please re-power-cycle.";
  logs_.emplace(log);
  return false;
}

bool GkcInterface::shutdown(const uint32_t & timeout_ms)
{
  if (current_state_ != GkcLifecycle::Active && current_state_ != GkcLifecycle::Inactive) {
    auto log = LogPacket();
    log.level = LogPacket::Severity::WARNING;
    log.what = "GKC can only be shutdown in active or inactive state. Current state is " +
      std::to_string(current_state_) + ".";
    logs_.emplace(log);
    return false;
  }
  bool succeeded = try_change_state(GkcLifecycle::Emergency, timeout_ms);
  succeeded &= send_shutdown();
  return succeeded;
}

const SensorGkcPacket * GkcInterface::get_sensors() const
{
  return sensors_.get();
}

GkcLifecycle GkcInterface::get_state() const
{
  return current_state_;
}

std::shared_ptr<LogPacket> GkcInterface::get_next_log()
{
  if (!logs_.size()) {
    return nullptr;
  }
  auto log = std::make_shared<LogPacket>(logs_.front());
  logs_.pop();
  return log;
}

bool GkcInterface::try_change_state(const GkcLifecycle & target_state, const uint32_t & timeout_ms)
{
  if (!comm_ || !comm_->is_open()) {
    return false;
  }

  auto activate_packet = StateTransitionGkcPacket();
  activate_packet.requested_state = static_cast<uint8_t>(target_state);

  auto sent = static_cast<bool>(comm_->send(*factory_->Send(activate_packet)));
  if (!sent) {
    return false;
  }

  // Wait for deactivation on the MCU
  std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));

  // Check state code to determine if state transition was successful
  return current_state_ == target_state;
}

void GkcInterface::stream_heartbeats()
{
  static constexpr uint32_t HEARTBEAT_INTERVAL_MS = 1000;
  auto hb = HeartbeatGkcPacket();
  hb.rolling_counter = 0;
  while (comm_ && comm_->is_open()) {
    comm_->send(*factory_->Send(hb));
    ++hb.rolling_counter;
    std::this_thread::sleep_for(std::chrono::milliseconds(HEARTBEAT_INTERVAL_MS));
  }
}

bool GkcInterface::send_handshake()
{
  if (!comm_ || !comm_->is_open()) {
    return false;
  }
  auto handshake_packet = Handshake1GkcPacket();
  handshake_packet.seq_number = static_cast<uint32_t>(std::rand());
  handshake_number = std::make_unique<uint32_t>(handshake_packet.seq_number);
  return static_cast<bool>(comm_->send(*factory_->Send(handshake_packet)));
}

bool GkcInterface::send_shutdown()
{
  if (!comm_ || !comm_->is_open()) {
    return false;
  }
  auto shutdown_packet = Shutdown1GkcPacket();
  shutdown_packet.seq_number = static_cast<uint32_t>(std::rand());
  shutdown_number = std::make_unique<uint32_t>(shutdown_packet.seq_number);
  return static_cast<bool>(comm_->send(*factory_->Send(shutdown_packet)));
}

bool GkcInterface::send_firmware_version_request()
{
  if (!comm_ || !comm_->is_open()) {
    return false;
  }
  auto packet = GetFirmwareVersionGkcPacket();
  return static_cast<bool>(comm_->send(*factory_->Send(packet)));
}

void GkcInterface::receive(const GkcBuffer & buffer)
{
  factory_->Receive(buffer);
}

void GkcInterface::packet_callback(const Handshake1GkcPacket & packet)
{
  (void)packet;
}
void GkcInterface::packet_callback(const Handshake2GkcPacket & packet)
{
  if (!handshake_number) {
    handshake_good_ = false;
    throw std::runtime_error("Handshake #2 received, but no handshake #1 was initiated before.");
  } else if (*handshake_number + 1 != packet.seq_number) {
    auto log = LogPacket();
    log.level = LogPacket::Severity::WARNING;
    log.what = "Handshake #2 received, but sequence number does not match.";
    logs_.emplace(log);
    handshake_good_ = false;
    return;
  }

  // Handshake is good.
  auto log = LogPacket();
  log.level = LogPacket::Severity::INFO;
  log.what = "Received valid handshake from GKC.";
  logs_.emplace(log);
  handshake_good_ = true;
}

void GkcInterface::packet_callback(const GetFirmwareVersionGkcPacket & packet)
{
  (void)packet;
}

void GkcInterface::packet_callback(const FirmwareVersionGkcPacket & packet)
{
  if (packet.major != GkcPacketLibVersion::MAJOR ||
    packet.minor != GkcPacketLibVersion::MINOR)
  {
    std::runtime_error(
      "GKC packet library version mismatch. MCU has version " +
      std::to_string(packet.major) + "." + std::to_string(packet.minor) +
      " whereas this PC has version " +
      std::to_string(GkcPacketLibVersion::MAJOR) + "." +
      std::to_string(GkcPacketLibVersion::MINOR) + ".");
  }

  if (packet.patch != GkcPacketLibVersion::PATCH) {
    auto log = LogPacket();
    log.level = LogPacket::Severity::WARNING;
    log.what = "GKC packet library version: patch number mismatch.";
    logs_.emplace(log);
  } else {
    auto log = LogPacket();
    log.level = LogPacket::Severity::INFO;
    log.what = "GKC packet library version matched.";
    logs_.emplace(log);
  }
}

void GkcInterface::packet_callback(const ResetMcuGkcPacket & packet)
{
  (void)packet;
}

void GkcInterface::packet_callback(const HeartbeatGkcPacket & packet)
{
  // TODO(haoru): handle heartbeat
  if (initialized_) {
    current_state_ = static_cast<GkcLifecycle>(packet.state);
  }
}

void GkcInterface::packet_callback(const ConfigGkcPacket & packet)
{
  (void)packet;
}

void GkcInterface::packet_callback(const StateTransitionGkcPacket & packet)
{
  (void)packet;
}

void GkcInterface::packet_callback(const ControlGkcPacket & packet)
{
  (void)packet;
}

void GkcInterface::packet_callback(const SensorGkcPacket & packet)
{
  if (handshake_good_) {
    sensors_ = std::make_unique<SensorGkcPacket>(packet);
  }
}

void GkcInterface::packet_callback(const Shutdown1GkcPacket & packet)
{
  (void)packet;
}

void GkcInterface::packet_callback(const Shutdown2GkcPacket & packet)
{
  if (!shutdown_number) {
    throw std::runtime_error("Shutdown #2 received, but no shutdown #1 was initiated before.");
  } else if (*handshake_number + 1 != packet.seq_number) {
    auto log = LogPacket();
    log.level = LogPacket::Severity::WARNING;
    log.what = "Handshake #2 received, but sequence number does not match. Retrying.";
    logs_.emplace(log);
    send_shutdown();
    return;
  }
}

void GkcInterface::packet_callback(const LogPacket & packet)
{
  logs_.emplace(packet);
}
}  // namespace gkc
}  // namespace tritonai
