/**
 * @file tai_gokart_controller_node.cpp
 * @author Haoru Xue (haoru.xue@autoware.org)
 * @brief
 * @version 0.1
 * @date 2022-02-07
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <string>
#include <memory>

#include "tai_gokart_controller/tai_gokart_controller_node.hpp"

namespace tritonai
{
namespace gkc
{
using std::placeholders::_1;

GkcNode::GkcNode(const rclcpp::NodeOptions & options)
: rclcpp_lifecycle::LifecycleNode("gkc_node", options)
{
  configs_ = ConfigList{
    Config{"comm_type", Configurable(declare_parameter<std::string>("comm_type", "serial"))},
    Config{"serial_port",
      Configurable(declare_parameter<std::string>("serial.port", "/dev/ttyACM0"))},
    Config{"baud_rate", Configurable{.integer = declare_parameter<int64_t>(
          "serial.baud_rate",
          115200)}},
  };
  interface_ = std::make_unique<GkcInterface>(configs_);
}

LifecycleNodeInterface::CallbackReturn GkcNode::on_configure(
  const rclcpp_lifecycle::State & previous_state)
{
  static bool first_time = true;
  static auto pkt = ConfigGkcPacket();
  if (first_time) {
    double sensor_pub_interval = 1.0 / declare_parameter<uint32_t>("sensor_pub_hz", 100);
    state_pub_ = create_publisher<GkcState>("gkc_state", rclcpp::QoS{10});
    cmd_sub_ =
      create_subscription<GkcCommand>(
      "gkc_cmd", rclcpp::QoS{10},
      std::bind(&GkcNode::cmd_callback, this, _1));
    state_pub_timer_ = create_timer(
      this, get_clock(), rclcpp::duration<float>(sensor_pub_interval), [this] {
        state_pub_timer_callback();
      });
    pkt.values.max_steering_left = declare_parameter<float>("max_steering_left", 0.524);
    pkt.values.max_steering_right = declare_parameter<float>("max_steering_right", -0.524);
    pkt.values.neutral_steering = declare_parameter<float>("neutral_steering", 0.0);
    pkt.values.max_throttle = declare_parameter<float>("max_throttle", 1.0);
    pkt.values.min_throttle = declare_parameter<float>("min_throttle", 0.1);
    pkt.values.zero_throttle = declare_parameter<float>("zero_throttle", 0.0);
    pkt.values.max_brake = declare_parameter<float>("max_brake", 2000.0);
    pkt.values.min_brake = declare_parameter<float>("min_brake", 200.0);
    pkt.values.min_brake = declare_parameter<float>("min_brake", 0.0);
    pkt.values.control_timeout_ms = declare_parameter<int64_t>("control_timeout_ms", 100);
    pkt.values.comm_timeout_ms = declare_parameter<int64_t>("comm_timeout_ms", 100);
    pkt.values.sensor_timeout_ms = declare_parameter<int64_t>("sensor_timeout_ms", 100);
    first_time = false;
  }
  RCLCPP_INFO(get_logger(), "Sending configuration to the MCU.");
  static constexpr uint32_t CONFIGURE_WAIT_MS = 100;
  static constexpr uint32_t MAX_INITIALIZE_WAIT_S = 60;
  if (interface_->initialize(pkt, CONFIGURE_WAIT_MS)) {
    RCLCPP_INFO(
      get_logger(), "MCU is initializing. Waiting for a max of %d second before timeout...",
      MAX_INITIALIZE_WAIT_S);
    auto start_time = get_clock()->now();
    while ((get_clock()->now() - start_time).seconds() < MAX_INITIALIZE_WAIT_S) {
      if (interface_->get_state() == GkcLifecycle::Inactive) {
        RCLCPP_INFO(get_logger(), "MCU is initialized and in inactive state.");
        return LifecycleNodeInterface::CallbackReturn::SUCCESS;
      }
      std::this_thread::sleep_for(std::chrono::duration<double>(1.0));
    }
    RCLCPP_ERROR(get_logger(), "MCU initialization timeout.");
  } else {
    RCLCPP_ERROR(get_logger(), "Failed to send configuration to the MCU.");
  }
  return LifecycleNodeInterface::CallbackReturn::FAILURE;
}

LifecycleNodeInterface::CallbackReturn GkcNode::on_activate(
  const rclcpp_lifecycle::State & previous_state)
{
  static constexpr uint32_t WAIT_MS = 100;
  if (interface_->activate(WAIT_MS)) {
    RCLCPP_WARN(get_logger(), "!!! VEHICLE IS ALIVE !!!");
    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  } else {
    RCLCPP_WARN(get_logger(), "Failed to activate vehicle. Maybe in emergency stop?");
    return LifecycleNodeInterface::CallbackReturn::FAILURE;
  }
}

LifecycleNodeInterface::CallbackReturn GkcNode::on_deactivate(
  const rclcpp_lifecycle::State & previous_state)
{
  static constexpr uint32_t WAIT_MS = 100;
  if (interface_->activate(WAIT_MS)) {
    RCLCPP_WARN(get_logger(), "VEHICLE IS PAUSED");
    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  } else {
    RCLCPP_WARN(
      get_logger(),
      "!!! VEHICLE STILL ALIVE !!! Failed to pause vehicle. This may cause emergency stop.");
    return LifecycleNodeInterface::CallbackReturn::FAILURE;
  }
}

LifecycleNodeInterface::CallbackReturn GkcNode::on_cleanup(
  const rclcpp_lifecycle::State & previous_state)
{
  static constexpr uint32_t WAIT_MS = 100;
  RCLCPP_INFO(get_logger(), "Attempting to bring the vehicle out of emergency mode.");
  if (interface_->release_emergency_stop(WAIT_MS)) {
    RCLCPP_INFO(get_logger(), "Done. Vehicle is now in uninitialized state.");
    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  } else {
    RCLCPP_INFO(get_logger(), "Failed. Vehicle is still in emergency mode.");
    return LifecycleNodeInterface::CallbackReturn::FAILURE;
  }
}

LifecycleNodeInterface::CallbackReturn GkcNode::on_shutdown(
  const rclcpp_lifecycle::State & previous_state)
{
  static constexpr uint32_t WAIT_MS = 100;
  if (interface_->shutdown(WAIT_MS)) {
    RCLCPP_INFO(get_logger(), "Vehicle shutdown.");
    return LifecycleNodeInterface::CallbackReturn::SUCCESS;
  } else {
    RCLCPP_INFO(get_logger(), "Vehicle failed to shutdown. It should now go to emergency mode.");
    return LifecycleNodeInterface::CallbackReturn::FAILURE;
  }
}

LifecycleNodeInterface::CallbackReturn GkcNode::on_error(
  const rclcpp_lifecycle::State & previous_state)
{
  if (!interface_)
  {
    RCLCPP_FATAL(get_logger(), "Communication to MCU not established. Exiting.");
    return LifecycleNodeInterface::CallbackReturn::FAILURE;
  }

  if (interface_->get_state() == GkcLifecycle::Uninitialized)
  {
    RCLCPP_FATAL(get_logger(), "Error raised in uninitialized state. Exiting.");
    return LifecycleNodeInterface::CallbackReturn::FAILURE;
  }

  static constexpr uint32_t WAIT_MS = 100;
  do {
    RCLCPP_WARN(get_logger(), "!!! E STOP REQUESTED !!!");
  } while (!interface_->emergency_stop(WAIT_MS));
  // TODO(haoru): issue: error is not a steady state
  return LifecycleNodeInterface::CallbackReturn::SUCCESS;
}

void GkcNode::cmd_callback(const GkcCommand::SharedPtr cmd_msg)
{
  auto pkt = ControlGkcPacket();
  pkt.throttle = cmd_msg->throttle;
  pkt.steering = cmd_msg->steering;
  pkt.brake = cmd_msg->brake;
  if (!interface_->send_control(pkt)) {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 500, "Failed to send control.");
  }
  if (cmd_msg->emergency_stop) {
    throw std::runtime_error("Emergency stop is requested by a command publisher.");
  }
}

void GkcNode::state_pub_timer_callback()
{
if (interface_ && state_pub_)
{
  GkcState state = GkcState();
  const auto & vals = interface_->get_sensors().values;
  state.amperage = vals.amperage;
  state.brake_pressure = vals.brake_pressure;
  state.fault_brake = vals.fault_brake;
  state.fault_error = vals.fault_error;
  state.fault_fatal = vals.fault_fatal;
  state.fault_info = vals.fault_info;
  state.fault_steering = vals.fault_steering;
  state.fault_throttle = vals.fault_throttle;
  state.fault_warning = vals.fault_warning;
  state.servo_angle_rad = vals.servo_angle_rad;
  state.steering_angle_rad = vals.steering_angle_rad;
  state.throttle_pos = vals.throttle_pos;
  state.voltage = vals.voltage;
  state.wheel_speed_fl = vals.wheel_speed_fl;
  state.wheel_speed_fr = vals.wheel_speed_fr;
  state.wheel_speed_rl = vals.wheel_speed_rl;
  state.wheel_speed_rr = vals.wheel_speed_rr;
  state.state = static_cast<uint8_t>(interface_->get_state());
  state.stamp = get_clock()->now();
  state_pub_->publish(state);
}

// Dump the logs off the interface
while (const auto log = interface_->get_next_log())
{
  switch(log->level)
  {
    case LogPacket::Severity::INFO:
      RCLCPP_INFO(get_logger(), log->what.c_str());
      break;
    case LogPacket::Severity::WARNING:
    RCLCPP_WARN(get_logger(), log->what.c_str());
      break;
    case LogPacket::Severity::ERROR:
    RCLCPP_ERROR(get_logger(), log->what.c_str());
      break;
    case LogPacket::Severity::FATAL:
    RCLCPP_FATAL(get_logger(), log->what.c_str());
      break;
  }
}
}
}  // namespace gkc
}  // namespace tritonai
