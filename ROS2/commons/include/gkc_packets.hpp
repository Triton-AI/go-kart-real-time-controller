/**
 * @file gkc_packets.hpp
 * @author Haoru Xue (hxue@ucsd.edu)
 * @brief Packet structures
 * @version 0.1
 * @date 2021-10-29
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef GKC_PACKETS_HPP
#define GKC_PACKETS_HPP

#include <iostream>
#include <memory>
#include <optional>
#include <vector>

namespace tritonai {
namespace gkc {

using GkcBuffer = std::vector<uint8_t>;
class RawGkcPacket {
 public:
  RawGkcPacket();

  /**
   * @brief Construct a new Raw Gkc Packet with payload
   *
   * @param payload payload of the packet
   */
  RawGkcPacket(const GkcBuffer& payload);
  RawGkcPacket(const GkcPacket& packet);
  RawGkcPacket& operator=(const RawGkcPacket& packet);
  const static uint8_t START_BYTE = 0x02;
  const static uint8_t END_BYTE = 0x03;

  uint8_t payload_size;
  uint16_t checksum;
  GkcBuffer payload;

  typedef std::shared_ptr<RawGkcPacket> SharedPtr;
  typedef std::unique_ptr<RawGkcPacket> UniquePtr;
};

class GkcPacket {
 public:
  typedef std::shared_ptr<GkcPacket> SharedPtr;
  typedef std::unique_ptr<GkcPacket> UniquePtr;
  uint64_t timestamp = 0;

  virtual RawGkcPacket::SharedPtr encode() = 0;
  virtual void decode(const RawGkcPacket& raw) = 0;
};

class Handshake1GkcPacket : public GkcPacket {
 public:
  uint32_t seq_number = 0;
  const static uint8_t FIRST_BYTE = 0x4;
  RawGkcPacket::SharedPtr encode();
  void decode(const RawGkcPacket& raw);
};

class Handshake2GkcPacket : public GkcPacket {
 public:
  uint32_t seq_number = 0;
  const static uint8_t FIRST_BYTE = 0x5;
  RawGkcPacket::SharedPtr encode();
  void decode(const RawGkcPacket& raw);
};

class GetFirmwareVersionGkcPacket : public GkcPacket {
 public:
  const static uint8_t FIRST_BYTE = 0x6;
  RawGkcPacket::SharedPtr encode();
  void decode(const RawGkcPacket& raw);
};

class FirmwareVersionGkcPacket : public GkcPacket {
 public:
  const static uint8_t FIRST_BYTE = 0x7;
  uint8_t major = 0;
  uint8_t minor = 0;
  uint8_t patch = 0;
  RawGkcPacket::SharedPtr encode();
  void decode(const RawGkcPacket& raw);
};

class ResetMcuGkcPacket : public GkcPacket {
 public:
  const static uint8_t FIRST_BYTE = 0x255;
  uint32_t magic_number = 0;
  RawGkcPacket::SharedPtr encode();
  void decode(const RawGkcPacket& raw);
};

class HeartbeatGkcPacket : public GkcPacket {
 public:
  const static uint8_t FIRST_BYTE = 0x252;
  uint8_t rolling_counter = 0;
  RawGkcPacket::SharedPtr encode();
  void decode(const RawGkcPacket& raw);
};

class GkcPacketUtils {
 public:
  /**
   * @brief calculate CRC-16 checksum
   * (https://github.com/vedderb/bldc/blob/master/crc.c)
   *
   * @param payload payload used to calculate checksum
   * @return uint16_t CRC-16 checksum
   */
  static uint16_t calc_crc16(const GkcBuffer& payload);
  static int64_t get_timestamp();

  template <typename T>
  static GkcPacket::SharedPtr CreatePacket() {
    GkcPacket::SharedPtr packet_ptr = std::make_shared<GkcPacket>(T);
    packet_ptr->timestamp = get_timestamp();
    return packet_ptr;
  }

  template <typename T>
  static void write_to_buffer(GkcBuffer::iterator where, const T & to_write){
  uint8_t* num_start = static_cast<uint8_t*>(static_cast<void*>(&to_write));
  std::copy(num_start, num_start + sizeof(T), where);
  }

  template <typename T>
  static void read_from_buffer(GkcBuffer::const_iterator where, T & to_read){
    to_read = *static_cast<T*>(static_cast<void*>(&(*where)));
  }
};

}  // namespace gkc
}  // namespace tritonai
#endif  // GKC_PACKETS_HPP