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

class ICommInterface;

/**
 * @brief An interface to receive content from low level communication implementation
 *
 */
class ICommRecvHandler
{
public:
  friend ICommInterface;
  ICommRecvHandler() {}
  virtual ~ICommRecvHandler() {}
  virtual void receive(const GkcBuffer & buffer) = 0;
};

/**
 * @brief An interface for low level communication implementation
 *
 */
class ICommInterface
{
public:
  typedef std::shared_ptr<ICommInterface> SharedPtr;
  typedef std::unique_ptr<ICommInterface> UniquePtr;

  ICommInterface() = delete;

  /**
   * @brief Initialize the interface with a receive handler
   *
   * @param handler receive handler
   */
  explicit ICommInterface(ICommRecvHandler * handler) {handler_ = handler;}

  /**
   * @brief register a different receive handler than the one assigned at construction
   *
   * @param handler receive handler
   */
  virtual void register_handler(ICommRecvHandler * handler)
  {
    handler_ = handler;
  }

  virtual ~ICommInterface() {}

  /**
   * @brief Configure the interface
   *
   * @param configs a map of configurable names and values
   * @return true if success; false if failed
   */
  virtual bool configure(const ConfigList & configs) = 0;

  /**
   * @brief Open the interface after configuration
   *
   * @return true if success; false if failed
   */
  virtual bool open() = 0;

  /**
   * @brief If the interface has been opened and is ready for IO operation
   */
  virtual bool is_open() = 0;

  /**
   * @brief Close the interface and release resources
   *
   * @return true if success; false if failed
   */
  virtual bool close() = 0;

  /**
   * @brief Send a buffer
   *
   * @param buffer to send
   * @return size_t number of bytes sent
   */
  virtual size_t send(const GkcBuffer & buffer) = 0;

  /**
   * @brief Get the interface type
   */
  virtual CommIO get_io_type() = 0;

protected:
  ICommRecvHandler * handler_;
};

class CommUtils
{
public:
  /**
  * @brief helper for creating implementations of `ICommInterface`
  *
  * @tparam T an implementation of `ICommInterface`
  * @param handler receive handler to be linked to the interface
  * @return ICommInterface::SharedPtr a new interface instance
  */
  template<typename T>
  static ICommInterface::SharedPtr CreateCommInterface(ICommRecvHandler * handler)
  {
    return std::make_shared<T>(handler);
  }
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
