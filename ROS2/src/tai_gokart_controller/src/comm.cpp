/**
 * @file comm.cpp
 * @author Haoru Xue (haoru.xue@autoware.org)
 * @brief
 * @version 0.1
 * @date 2022-02-06
 *
 * @copyright Copyright 2022 Triton AI
 *
 */

#include <string>
#include <thread>
#include <memory>

#include "tai_gokart_controller/comm.hpp"

namespace tritonai
{
namespace gkc
{
SerialInterface::SerialInterface(ICommRecvHandler * handler)
: ICommInterface(handler),
  owned_ctx{new drivers::common::IoContext(2)},
  driver_{new drivers::serial_driver::SerialDriver(*owned_ctx)}
{
}

SerialInterface::~SerialInterface()
{
  if (driver_ && driver_->port()->is_open()) {
    running_ = false;
    close();
    if (recv_thread && recv_thread->joinable()) {
      recv_thread->join();
    }
  }
}

bool SerialInterface::configure(const ConfigList & configs)
{
  std::string serial_port = &(configs.at("serial_port").string[0]);
  uint32_t baud_rate = static_cast<uint32_t>(configs.at("baud_rate").integer);
  auto fc = drivers::serial_driver::FlowControl::HARDWARE;
  auto pt = drivers::serial_driver::Parity::NONE;
  auto sb = drivers::serial_driver::StopBits::ONE;
  driver_->init_port(serial_port, drivers::serial_driver::SerialPortConfig(baud_rate, fc, pt, sb));
  return true;
}

bool SerialInterface::open()
{
  if (!driver_) {
    return false;
  }
  if (!driver_->port()->is_open()) {
    driver_->port()->open();
  }
  if (!driver_->port()->is_open()) {
    return false;
  }
  // Start recv thread
  recv_thread = std::unique_ptr<std::thread>(new std::thread(&SerialInterface::recv, this));
  return true;
}

bool SerialInterface::is_open()
{
  if (!driver_) {
    return false;
  }
  return driver_->port()->is_open();
}

bool SerialInterface::close()
{
  if (!driver_) {
    return false;
  }
  if (driver_->port()->is_open()) {
    driver_->port()->close();
  }
  return !driver_->port()->is_open();
}

size_t SerialInterface::send(const GkcBuffer & buffer)
{
  if (driver_ && driver_->port()->is_open()) {
    driver_->port()->async_send(buffer);
    return buffer.size();
  }
  return 0;
}

void SerialInterface::recv()
{
  auto buffer = GkcBuffer(2048, 0);
  while (running_ && driver_->port()->is_open()) {
    auto bytes_read = driver_->port()->receive(buffer);
    if (bytes_read > 0) {
      handler_->receive(GkcBuffer(buffer.begin(), buffer.begin() + bytes_read));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

CommIO SerialInterface::get_io_type()
{
  return CommIO::Serial;
}
}  // namespace gkc
}  // namespace tritonai
