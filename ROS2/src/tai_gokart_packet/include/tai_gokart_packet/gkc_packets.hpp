/**
 * @file gkc_packets.hpp
 * @author Haoru Xue (hxue@ucsd.edu)
 * @brief Packet structures
 * @version 0.1
 * @date 2021-10-29
 *
 * @copyright Copyright (c) 2021 [Triton AI]
 *
 */

#ifndef TAI_GOKART_PACKET__GKC_PACKETS_HPP_
#define TAI_GOKART_PACKET__GKC_PACKETS_HPP_

#include <optional>
#include <iostream>
#include <memory>
#include <vector>

#include "tai_gokart_packet/gkc_packet_subscriber.hpp"

namespace tritonai
{
namespace gkc
{

using GkcBuffer = std::vector<uint8_t>;
class RawGkcPacket
{
public:
  const static uint8_t START_BYTE = 0x02;
  const static uint8_t END_BYTE = 0x03;

  RawGkcPacket();

  /**
   * @brief Construct a new Raw Gkc Packet with payload and calculates checksum
   *
   * @param payload payload of the packet
   */

  explicit RawGkcPacket(const GkcBuffer & payload);
  RawGkcPacket(const RawGkcPacket & packet);
  RawGkcPacket & operator=(const RawGkcPacket & packet);

  std::shared_ptr<GkcBuffer> encode();

  uint8_t payload_size;
  uint16_t checksum;
  GkcBuffer payload;

  typedef std::shared_ptr<RawGkcPacket> SharedPtr;
  typedef std::unique_ptr<RawGkcPacket> UniquePtr;
};

class GkcPacket
{
public:
  typedef std::shared_ptr<GkcPacket> SharedPtr;
  typedef std::unique_ptr<GkcPacket> UniquePtr;
  uint64_t timestamp = 0;

  virtual RawGkcPacket::SharedPtr encode() const;
  virtual void decode(const RawGkcPacket & raw);
  virtual void publish(GkcPacketSubscriber & sub);
};

class Handshake1GkcPacket : public GkcPacket
{
public:
  const static uint8_t FIRST_BYTE = 0x4;
  uint32_t seq_number = 0;
  RawGkcPacket::SharedPtr encode() const;
  void decode(const RawGkcPacket & raw);
  void publish(GkcPacketSubscriber & sub) {return sub.packet_callback(*this);}
};

class Handshake2GkcPacket : public GkcPacket
{
public:
  const static uint8_t FIRST_BYTE = 0x5;
  uint32_t seq_number = 0;
  RawGkcPacket::SharedPtr encode() const;
  void decode(const RawGkcPacket & raw);
  void publish(GkcPacketSubscriber & sub) {return sub.packet_callback(*this);}
};

class GetFirmwareVersionGkcPacket : public GkcPacket
{
public:
  const static uint8_t FIRST_BYTE = 0x6;
  RawGkcPacket::SharedPtr encode() const;
  void decode(const RawGkcPacket & raw);
  void publish(GkcPacketSubscriber & sub) {return sub.packet_callback(*this);}
};

class FirmwareVersionGkcPacket : public GkcPacket
{
public:
  const static uint8_t FIRST_BYTE = 0x7;
  uint8_t major = 0;
  uint8_t minor = 0;
  uint8_t patch = 0;
  RawGkcPacket::SharedPtr encode() const;
  void decode(const RawGkcPacket & raw);
  void publish(GkcPacketSubscriber & sub) {return sub.packet_callback(*this);}
};

class ResetMcuGkcPacket : public GkcPacket
{
public:
  const static uint8_t FIRST_BYTE = 0xFF;
  uint32_t magic_number = 0;
  RawGkcPacket::SharedPtr encode() const;
  void decode(const RawGkcPacket & raw);
  void publish(GkcPacketSubscriber & sub) {return sub.packet_callback(*this);}
};

class HeartbeatGkcPacket : public GkcPacket
{
public:
  const static uint8_t FIRST_BYTE = 0xAA;
  uint8_t rolling_counter = 0;
  RawGkcPacket::SharedPtr encode() const;
  void decode(const RawGkcPacket & raw);
  void publish(GkcPacketSubscriber & sub) {return sub.packet_callback(*this);}
};

class GkcPacketUtils
{
public:
  /**
   * @brief calculate CRC-16 checksum
   * (https://github.com/vedderb/bldc/blob/master/crc.c)
   *
   * @param payload payload used to calculate checksum
   * @return uint16_t CRC-16 checksum
   */
  static uint16_t calc_crc16(const GkcBuffer & payload);
  static int64_t get_timestamp();
  static void debug_cout(std::string str);

  template<typename T>
  static GkcPacket::SharedPtr CreatePacket()
  {
    T t;
    GkcPacket::SharedPtr packet_ptr = std::make_shared<GkcPacket>(t);
    packet_ptr->timestamp = get_timestamp();
    return packet_ptr;
  }

  template<typename T>
  static void write_to_buffer(GkcBuffer::iterator where, const T & to_write)
  {
    const uint8_t * num_start =
      static_cast<const uint8_t *>(static_cast<const void *>(&to_write));
    std::copy(num_start, num_start + sizeof(T), where);
  }

  template<typename T>
  static void read_from_buffer(GkcBuffer::const_iterator where, T & to_read)
  {
    to_read = *static_cast<const T *>(static_cast<const void *>(&(*where)));
  }
};


}  // namespace gkc
}  // namespace tritonai
#endif  // TAI_GOKART_PACKET__GKC_PACKETS_HPP_
