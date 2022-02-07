/**
 * @file tai_gokart_interface.hpp
 * @author Haoru Xue (haoru.xue@autoware.org)
 * @brief
 * @version 0.1
 * @date 2022-02-06
 *
 * @copyright Copyright 2022 Triton AI
 *
 */
#ifndef TAI_GOKART_CONTROLLER__TAI_GOKART_INTERFACE_HPP_
#define TAI_GOKART_CONTROLLER__TAI_GOKART_INTERFACE_HPP_

#include <queue>
#include <string>
#include <memory>
#include <unordered_map>

#include "tai_gokart_packet/gkc_packets.hpp"
#include "tai_gokart_packet/gkc_packet_factory.hpp"
#include "tai_gokart_packet/gkc_packet_utils.hpp"
#include "tai_gokart_packet/gkc_packet_subscriber.hpp"

#include "tai_gokart_controller/comm.hpp"
#include "tai_gokart_controller/config.hpp"

namespace tritonai
{
namespace gkc
{
class GkcInterface : public GkcPacketSubscriber, public ICommRecvHandler
{
public:
  GkcInterface() = delete;
  explicit GkcInterface(const ConfigList & configs);
  ~GkcInterface();

  // APIs
  bool send_control(const ControlGkcPacket & control_packet);
  bool initialize(const ConfigGkcPacket & config_packet, const uint32_t & timeout_ms);
  bool activate(const uint32_t & timeout_ms);
  bool deactivate(const uint32_t & timeout_ms);
  bool emergency_stop(const uint32_t & timeout_ms);
  bool release_emergency_stop(const uint32_t & timeout_ms);
  bool shutdown(const uint32_t & timeout_ms);
  const SensorGkcPacket & get_sensors() const;
  GkcState get_state() const;
  LogPacket::SharedPtr get_next_log();

  // ICommRecvHandler
  void receive(const GkcBuffer & buffer);

  // GkcPacketSubscriber
  void packet_callback(const Handshake1GkcPacket & packet);
  void packet_callback(const Handshake2GkcPacket & packet);
  void packet_callback(const GetFirmwareVersionGkcPacket & packet);
  void packet_callback(const FirmwareVersionGkcPacket & packet);
  void packet_callback(const ResetMcuGkcPacket & packet);
  void packet_callback(const HeartbeatGkcPacket & packet);
  void packet_callback(const ConfigGkcPacket & packet);
  void packet_callback(const StateTransitionGkcPacket & packet);
  void packet_callback(const ControlGkcPacket & packet);
  void packet_callback(const SensorGkcPacket & packet);
  void packet_callback(const Shutdown1GkcPacket & packet);
  void packet_callback(const Shutdown2GkcPacket & packet);
  void packet_callback(const LogPacket & packet);

protected:
  std::queue<LogPacket> logs_ {};
  std::shared_ptr<ICommInterface> comm_ {};
  std::unique_ptr<std::thread> heartbeat_thread {};
  std::unique_ptr<GkcPacketFactory> factory_ {};
  std::unique_ptr<SensorGkcPacket> sensors_ {};
  std::unique_ptr<uint32_t> handshake_number {};
  std::unique_ptr<uint32_t> shutdown_number {};

  GkcState current_state_ {GkcState::Uninitialized};

  // Inner working
  bool try_change_state(const GkcState & target_state, const uint32_t & timeout_ms);
  void stream_heartbeats();
  bool send_handshake();
  bool send_shutdown();
  bool send_firmware_version_request();

  typedef ICommInterface::SharedPtr (* Creator)(ICommRecvHandler * handler);
  const std::unordered_map<std::string, Creator> comm_lookup_ = {
    {"serial", CommUtils::CreateCommInterface<SerialInterface>}
  };
};
}  // namespace gkc
}  // namespace tritonai
#endif  // TAI_GOKART_CONTROLLER__TAI_GOKART_INTERFACE_HPP_
