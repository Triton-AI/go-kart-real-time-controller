/**
 * @file gkc_packet_factory.hpp
 * @author Haoru Xue (hxue@ucsd.edu)
 * @brief Encode and decode packets
 * @version 0.1
 * @date 2021-10-29
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef GKC_PACKET_FACTORY_HPP
#define GKC_PACKET_FACTORY_HPP

#include <gkc_packets.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace tritonai {
namespace gkc {
class GkcPacketFactory {
 public:
  GkcPacketFactory(void (*packet_callback)(GkcPacket*),
                   void (*debug)(std::string));

  void Receive(const GkcBuffer& buffer);

 private:
  void (*packet_callback)(GkcPacket*);
  typedef GkcPacket::SharedPtr (*Creator)();
  std::map<uint8_t, Creator> fb_lookup = {
      {Handshake1GkcPacket::FIRST_BYTE, GkcPacketUtils::CreatePacket<Handshake1GkcPacket>},
      {Handshake2GkcPacket::FIRST_BYTE, GkcPacketUtils::CreatePacket<Handshake2GkcPacket>},
      {GetFirmwareVersionGkcPacket::FIRST_BYTE, GkcPacketUtils::CreatePacket<GetFirmwareVersionGkcPacket>},
      {FirmwareVersionGkcPacket::FIRST_BYTE, GkcPacketUtils::CreatePacket<FirmwareVersionGkcPacket>},
      {ResetMcuGkcPacket::FIRST_BYTE, GkcPacketUtils::CreatePacket<ResetMcuGkcPacket>},
      {HeartbeatGkcPacket::FIRST_BYTE, GkcPacketUtils::CreatePacket<HeartbeatGkcPacket>},};
};

}  // namespace gkc
}  // namespace tritonai
#endif  // GKC_PACKET_FACTORY_HPP
