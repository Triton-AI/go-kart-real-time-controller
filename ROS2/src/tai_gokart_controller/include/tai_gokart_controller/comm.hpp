/**
 * @file comm.hpp
 * @author Haoru Xue (haoru.xue@autoware.org)
 * @brief
 * @version 0.1
 * @date 2022-02-06
 *
 * @copyright Copyright 2022 Triton AI
 *
 */

#ifndef TAI_GOKART_CONTROLLER__COMM_HPP_
#define TAI_GOKART_CONTROLLER__COMM_HPP_

#include <map>
#include <string>
#include <memory>
#include <queue>
#include <thread>

#include "serial_driver/serial_driver.hpp"

#include "tai_gokart_packet/gkc_packet_factory.hpp"
#include "tai_gokart_packet/gkc_packet_subscriber.hpp"
#include "tai_gokart_packet/gkc_packet_utils.hpp"
#include "tai_gokart_packet/gkc_packets.hpp"

#include "tai_gokart_controller/config.hpp"

namespace tritonai
{
namespace gkc
{
enum CommIO
{
  Serial = 0,
  Ethernet = 1,
  CAN = 2
};

class CommUtils
{
public:
  template<typename T>
  static ICommInterface::SharedPtr CreateCommInterface(ICommRecvHandler * handler)
  {
    return std::make_shared<T>(handler);
  }
};

class ICommRecvHandler
{
public:
  friend ICommInterface;
  ICommRecvHandler() {}
  virtual ~ICommRecvHandler() {}
  virtual void receive(const GkcBuffer & buffer) = 0;
};

class ICommInterface
{
public:
  typedef std::shared_ptr<ICommInterface> SharedPtr;
  typedef std::unique_ptr<ICommInterface> UniquePtr;
  ICommInterface() = delete;
  explicit ICommInterface(ICommRecvHandler * handler) {handler_ = handler;}
  virtual void register_handler(ICommRecvHandler * handler)
  {
    handler_ = handler;
  }
  virtual ~ICommInterface() {}
  virtual bool configure(const ConfigList & configs) = 0;
  virtual bool open() = 0;
  virtual bool is_open() = 0;
  virtual bool close() = 0;
  virtual size_t send(const GkcBuffer & buffer) = 0;
  virtual CommIO get_io_type() = 0;

protected:
  ICommRecvHandler * handler_;
};

class SerialInterface : public ICommInterface
{
public:
  SerialInterface() = delete;
  explicit SerialInterface(ICommRecvHandler * handler);
  ~SerialInterface();

  bool configure(const ConfigList & configs);
  bool open();
  bool is_open();
  bool close();
  size_t send(const GkcBuffer & buffer);
  CommIO get_io_type();

  void recv();

protected:
  std::unique_ptr<drivers::common::IoContext> owned_ctx {};
  std::unique_ptr<drivers::serial_driver::SerialDriver> driver_ {};
  std::unique_ptr<std::thread> recv_thread;
  bool running_ = true;
};
}  // namespace gkc
}  // namespace tritonai
#endif  // TAI_GOKART_CONTROLLER__COMM_HPP_
