/**
 * @file gkc_packet_factory.cpp
 * @author Haoru Xue (hxue@ucsd.edu)
 * @brief
 * @version 0.1
 * @date 2021-11-03
 *
 * @copyright Copyright (c) 2021 [Triton AI]
 *
 */

#include "tai_gokart_packet/gkc_packet_factory.hpp"
#include <cassert>
#include <memory>
#include <string>
#include <algorithm>
namespace tritonai
{
namespace gkc
{
GkcPacketFactory::GkcPacketFactory(GkcPacketSubscriber * sub, void(*debug)(std::string))
{
  this->_debug = debug;
  this->_sub = sub;
}

void GkcPacketFactory::Receive(const GkcBuffer & buffer)
{
  static const int MIN_PACKET_SIZE = 7;
  static int start_idx = 0;

  _buffer.reserve(_buffer.size() + buffer.size());
  std::copy(buffer.begin(), buffer.end(), _buffer.end());

  // Look for the start byte
  for (const auto & byte : _buffer) {
    if (byte == RawGkcPacket::START_BYTE) {
      break;
    }
    ++start_idx;
  }

  try {
    if (_buffer.size() - start_idx >= MIN_PACKET_SIZE) {
      uint16_t payload_size =
        *reinterpret_cast<uint16_t *>(&_buffer[start_idx + 1]);
      // Check packet completeness
      if ((_buffer[start_idx + 6 + payload_size] != RawGkcPacket::END_BYTE) |
        (payload_size < 1))
      {
        throw 42;
      }

      const GkcBuffer payload =
        GkcBuffer(
        _buffer.begin() + start_idx + 3,
        _buffer.begin() + start_idx + 3 + payload_size - 1);
      uint16_t checksum =
        *reinterpret_cast<uint16_t *>(&_buffer[start_idx + 3 + payload_size]);

      // Check checksum
      if (GkcPacketUtils::calc_crc16(payload) != checksum) {
        throw 42;
      }

      auto raw_packet = RawGkcPacket(payload);

      uint8_t fb = _buffer[start_idx + 3];
      auto packet = fb_lookup.find(fb)->second();
      packet->decode(raw_packet);
      packet->publish(*(this->_sub));

      start_idx = 0;
      _buffer.erase(_buffer.begin(), _buffer.begin() + 6 + payload_size);
    }
  } catch (...) {
  }
}

std::shared_ptr<GkcBuffer> GkcPacketFactory::Send(const GkcPacket::SharedPtr & packet)
{
  return packet->encode()->encode();
}
}  // namespace gkc
}  // namespace tritonai
