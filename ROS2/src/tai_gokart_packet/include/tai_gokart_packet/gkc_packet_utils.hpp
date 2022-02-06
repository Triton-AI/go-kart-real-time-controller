/**
 * @file gkc_packet_utils.hpp
 * @author Haoru Xue (haoru.xue@autoware.org)
 * @brief Utilities for gkc packet
 * @version 0.1
 * @date 2022-02-06
 *
 * @copyright Copyright (c) 2022 [Triton AI]
 *
 */

#ifndef TAI_GOKART_PACKET__GKC_PACKET_UTILS_HPP_
#define TAI_GOKART_PACKET__GKC_PACKET_UTILS_HPP_

#include <memory>
#include <vector>
#include <algorithm>
#include <string>

namespace tritonai
{
namespace gkc
{
using GkcBuffer = std::vector<uint8_t>;
class GkcPacket;
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
  static void debug_cout(std::string str);

  template<typename T>
  static std::shared_ptr<GkcPacket> CreatePacket()
  {
    auto packet_ptr = std::shared_ptr<GkcPacket>(new T);
    return packet_ptr;
  }

  /**
   * @brief Write some primitive types or struct to buffer
   *
   * @tparam T the datatype
   * @param where iterator to the start of destination
   * @param to_write value to write
   * @return GkcBuffer::iterator an iterator to the end of the copied content
   */
  template<typename T>
  static GkcBuffer::iterator write_to_buffer(GkcBuffer::iterator where, const T & to_write)
  {
    const uint8_t * start =
      static_cast<const uint8_t *>(static_cast<const void *>(&to_write));
    const auto end = start + sizeof(T);
    std::copy(start, end, where);
    return where + sizeof(T);
  }

  /**
   * @brief pointer version of `write_to_buffer()`
   *
   * @tparam T the datatype
   * @param where iterator to the start of destination
   * @param to_write value to write
   * @return uint8_t* a pointer to the end of the copied content
   */
  template<typename T>
  static uint8_t * write_to_buffer(const uint8_t * & where, const T & to_write)
  {
    const uint8_t * start =
      static_cast<const uint8_t *>(static_cast<const void *>(&to_write));
    const auto end = start + sizeof(T);
    std::copy(start, end, where);
    return end;
  }

  /**
   * @brief Read content from part of a buffer utilizing `sizeof(T)`
   *
   * @tparam T datatype to be read in
   * @param where where to start reading the content
   * @param to_read where to store the read content
   * @return GkcBuffer::const_iterator an iterator to the end of the read bytes in the buffer
   */
  template<typename T>
  static GkcBuffer::const_iterator read_from_buffer(GkcBuffer::const_iterator where, T & to_read)
  {
    // TODO(haoru): reinterpret_cast?
    to_read = *static_cast<const T *>(static_cast<const void *>(&(*where)));
    return where + sizeof(T);
  }

  /**
   * @brief pointer version of `read_from_buffer`
   *
   * @tparam T datatype to be read in
   * @param where where to start reading the content
   * @param to_read where to store the read content
   * @return const uint8_t* a pointer to the end of the read bytes in the buffer
   */
  template<typename T>
  static const uint8_t * read_from_buffer(const uint8_t * & where, T & to_read)
  {
    to_read = *static_cast<const T *>(static_cast<const void *>(where));
    return where + sizeof(T);
  }
};
}  // namespace gkc
}  // namespace tritonai
#endif  // TAI_GOKART_PACKET__GKC_PACKET_UTILS_HPP_
