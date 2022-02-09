/**
 * @file tai_gokart_controller_node.hpp
 * @author Haoru Xue (haoru.xue@autoware.org)
 * @brief
 * @version 0.1
 * @date 2022-02-07
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef TAI_GOKART_CONTROLLER__TAI_GOKART_CONTROLLER_NODE_HPP_
#define TAI_GOKART_CONTROLLER__TAI_GOKART_CONTROLLER_NODE_HPP_

#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"

#include "tai_gokart_msgs/msg/gkc_command.hpp"
#include "tai_gokart_msgs/msg/gkc_state.hpp"

#include "tai_gokart_controller/tai_gokart_interface.hpp"

namespace tritonai
{
namespace gkc
{
using tai_gokart_msgs::msg::GkcCommand;
using tai_gokart_msgs::msg::GkcState;
using rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface;

class GkcNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  explicit GkcNode(const rclcpp::NodeOptions & options);

  // LifecycleNode interface
  LifecycleNodeInterface::CallbackReturn on_configure(
    const rclcpp_lifecycle::State & previous_state);
  LifecycleNodeInterface::CallbackReturn on_activate(
    const rclcpp_lifecycle::State & previous_state);
  LifecycleNodeInterface::CallbackReturn on_deactivate(
    const rclcpp_lifecycle::State & previous_state);
  LifecycleNodeInterface::CallbackReturn on_cleanup(
    const rclcpp_lifecycle::State & previous_state);
  LifecycleNodeInterface::CallbackReturn on_shutdown(
    const rclcpp_lifecycle::State & previous_state);
  LifecycleNodeInterface::CallbackReturn on_error(
    const rclcpp_lifecycle::State & previous_state);

private:
  rclcpp::Publisher<GkcState>::SharedPtr state_pub_;
  rclcpp::Subscription<GkcCommand>::SharedPtr cmd_sub_;
  rclcpp::TimerBase::SharedPtr state_pub_timer_;
  std::unique_ptr<GkcInterface> interface_;
  ConfigList configs_;


  void cmd_callback(const GkcCommand::SharedPtr cmd_msg);
  void state_pub_timer_callback();
  void dump_logs();
};
}  // namespace gkc
}  // namespace tritonai
#endif  // TAI_GOKART_CONTROLLER__TAI_GOKART_CONTROLLER_NODE_HPP_
