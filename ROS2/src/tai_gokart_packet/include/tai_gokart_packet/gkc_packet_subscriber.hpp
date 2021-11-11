/**
 * @file gkc_packet_subscriber.hpp
 * @author Haoru Xue (hxue@ucsd.edu)
 * @brief 
 * @version 0.1
 * @date 2021-11-04
 * 
 * @copyright Copyright (c) 2021 [Triton AI]
 * 
 */

class Handshake1GkcPacket;
class Handshake2GkcPacket;
class GetFirmwareVersionGkcPacket;
class FirmwareVersionGkcPacket;
class ResetMcuGkcPacket;
class HeartbeatGkcPacket;
/**
 * @brief Subclass this to receive GkcPackets from GkcPacketFactory
 *
 */
class GkcPacketSubscriber
{
public:
  virtual void packet_callback(const Handshake1GkcPacket & packet) = 0;
  virtual void packet_callback(const Handshake2GkcPacket & packet) = 0;
  virtual void packet_callback(const GetFirmwareVersionGkcPacket & packet) = 0;
  virtual void packet_callback(const FirmwareVersionGkcPacket & packet) = 0;
  virtual void packet_callback(const ResetMcuGkcPacket & packet) = 0;
  virtual void packet_callback(const HeartbeatGkcPacket & packet) = 0;
};