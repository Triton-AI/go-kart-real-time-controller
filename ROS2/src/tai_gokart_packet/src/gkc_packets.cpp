/**
 * @file gkc_packets.cpp
 * @author Haoru Xue (hxue@ucsd.edu)
 * @brief Packet structures
 * @version 0.1
 * @date 2021-10-30
 *
 * @copyright Copyright (c) 2021 [Triton AI]
 *
 */
#include "tai_gokart_packet/gkc_packets.hpp"
#include <chrono>
#include <algorithm>
#include <memory>
#include <string>
namespace tritonai
{
namespace gkc
{

RawGkcPacket::RawGkcPacket()
: payload_size(0), checksum(0), payload(GkcBuffer()) {}

RawGkcPacket::RawGkcPacket(const GkcBuffer & payload)
{
  payload_size = payload.size();
  this->payload = GkcBuffer(payload_size, 0);
  std::copy(payload.begin(), payload.end(), this->payload.begin());
  checksum = GkcPacketUtils::calc_crc16(payload);
}

RawGkcPacket::RawGkcPacket(const RawGkcPacket & packet) {*this = packet;}

RawGkcPacket & RawGkcPacket::operator=(const RawGkcPacket & packet)
{
  payload_size = packet.payload_size;
  payload = GkcBuffer(payload_size, 0);
  std::copy(packet.payload.begin(), packet.payload.end(), payload.begin());
  checksum = packet.checksum;
  return *this;
}

std::shared_ptr<GkcBuffer> RawGkcPacket::encode()
{
  static constexpr uint8_t NUM_NON_PAYLOAD_BYTES = 5;
  auto buffer = std::make_unique<GkcBuffer>(payload_size + NUM_NON_PAYLOAD_BYTES, 0);
  auto pos_payload_size =
    GkcPacketUtils::write_to_buffer(buffer->begin(), static_cast<uint8_t>(START_BYTE));
  auto pos_payload = GkcPacketUtils::write_to_buffer(pos_payload_size, payload_size);
  auto pos_checksum = std::copy(payload.begin(), payload.end(), pos_payload);
  auto pos_end_byte = GkcPacketUtils::write_to_buffer(pos_checksum, checksum);
  auto pos_end = GkcPacketUtils::write_to_buffer(pos_end_byte, static_cast<uint8_t>(END_BYTE));
  if (pos_end != buffer->end()) {
    // Sanity check: encoding should use the entire buffer
    throw std::runtime_error(
            "Error when encoding raw gkc packet. Potential payload size mismatch.");
  }
  return buffer;
}

GkcPacket::~GkcPacket()
{
}

RawGkcPacket::SharedPtr GkcPacket::encode() const
{
  return std::make_shared<RawGkcPacket>();
}

void GkcPacket::decode(const RawGkcPacket & raw)
{
  (void)raw;
}

void GkcPacket::publish(GkcPacketSubscriber & sub)
{
  (void)sub;
}

uint16_t GkcPacketUtils::calc_crc16(const GkcBuffer & payload)
{
  static const uint16_t crc16_tab[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108,
    0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210,
    0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b,
    0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
    0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee,
    0xf5cf, 0xc5ac, 0xd58d, 0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6,
    0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d,
    0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5,
    0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc,
    0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4,
    0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
    0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13,
    0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,
    0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e,
    0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1,
    0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb,
    0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0,
    0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
    0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657,
    0x7676, 0x4615, 0x5634, 0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,
    0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882,
    0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e,
    0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07,
    0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f, 0xff3e, 0xcf5d,
    0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
    0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};

  uint16_t checksum = 0;
  for (const auto & byte : payload) {
    checksum = crc16_tab[(((checksum >> 8) ^ byte) & 0xFF)] ^ (checksum << 8);
  }
  return checksum;
}

void GkcPacketUtils::debug_cout(std::string str)
{
  std::cout << str << std::endl;
}

/*
Handshake 1
*/
RawGkcPacket::SharedPtr Handshake1GkcPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(5, 0);
  payload[0] = FIRST_BYTE;
  GkcPacketUtils::write_to_buffer<uint32_t>(payload.begin() + 1, seq_number);
  return std::make_unique<RawGkcPacket>(payload);
}

void Handshake1GkcPacket::decode(const RawGkcPacket & raw)
{
  GkcPacketUtils::read_from_buffer<uint32_t>(
    raw.payload.begin() + 1,
    seq_number);
}

/*
Handshake 2
*/
RawGkcPacket::SharedPtr Handshake2GkcPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(5, 0);
  payload[0] = FIRST_BYTE;
  GkcPacketUtils::write_to_buffer<uint32_t>(payload.begin() + 1, seq_number);
  return std::make_unique<RawGkcPacket>(payload);
}

void Handshake2GkcPacket::decode(const RawGkcPacket & raw)
{
  GkcPacketUtils::read_from_buffer<uint32_t>(
    raw.payload.begin() + 1,
    seq_number);
}

/*
Get firmware
*/
RawGkcPacket::SharedPtr GetFirmwareVersionGkcPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(1, 0);
  payload[0] = FIRST_BYTE;
  return std::make_unique<RawGkcPacket>(payload);
}

void GetFirmwareVersionGkcPacket::decode(const RawGkcPacket & raw)
{
  (void)raw;
}

/*
Firmware
*/
RawGkcPacket::SharedPtr FirmwareVersionGkcPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(4, 0);
  payload[0] = FIRST_BYTE;
  payload[1] = major;
  payload[2] = minor;
  payload[3] = patch;
  return std::make_unique<RawGkcPacket>(payload);
}

void FirmwareVersionGkcPacket::decode(const RawGkcPacket & raw)
{
  major = raw.payload[1];
  minor = raw.payload[2];
  patch = raw.payload[3];
}

/*
Reset IMU
*/
RawGkcPacket::SharedPtr ResetMcuGkcPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(5, 0);
  payload[0] = FIRST_BYTE;
  GkcPacketUtils::write_to_buffer<uint32_t>(payload.begin() + 1, magic_number);
  return std::make_unique<RawGkcPacket>(payload);
}

void ResetMcuGkcPacket::decode(const RawGkcPacket & raw)
{
  GkcPacketUtils::read_from_buffer<uint32_t>(
    raw.payload.begin() + 1,
    magic_number);
}

/*
Heartbeat
*/
RawGkcPacket::SharedPtr HeartbeatGkcPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(2, 0);
  payload[0] = FIRST_BYTE;
  payload[1] = rolling_counter;
  return std::make_unique<RawGkcPacket>(payload);
}

void HeartbeatGkcPacket::decode(const RawGkcPacket & raw)
{
  rolling_counter = raw.payload[1];
}

/*
Config
*/
RawGkcPacket::SharedPtr ConfigGkcPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(sizeof(Configurables) + 1, 0);
  payload[0] = FIRST_BYTE;
  GkcPacketUtils::write_to_buffer(payload.begin() + 1, values);
  return std::make_unique<RawGkcPacket>(payload);
}

void ConfigGkcPacket::decode(const RawGkcPacket & raw)
{
  GkcPacketUtils::read_from_buffer(
    raw.payload.begin() + 1,
    values);
}

/*
State Transition
*/
RawGkcPacket::SharedPtr StateTransitionGkcPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(2, 0);
  payload[0] = FIRST_BYTE;
  GkcPacketUtils::write_to_buffer(payload.begin() + 1, requested_state);
  return std::make_unique<RawGkcPacket>(payload);
}

void StateTransitionGkcPacket::decode(const RawGkcPacket & raw)
{
  GkcPacketUtils::read_from_buffer(
    raw.payload.begin() + 1,
    requested_state);
}

/*
Control
*/
RawGkcPacket::SharedPtr ControlGkcPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(13, 0);
  payload[0] = FIRST_BYTE;
  auto pos_steering = GkcPacketUtils::write_to_buffer(payload.begin() + 1, throttle);
  auto pos_brake = GkcPacketUtils::write_to_buffer(pos_steering, steering);
  GkcPacketUtils::write_to_buffer(pos_brake, brake);
  return std::make_unique<RawGkcPacket>(payload);
}

void ControlGkcPacket::decode(const RawGkcPacket & raw)
{
  auto pos_steering = GkcPacketUtils::read_from_buffer(raw.payload.begin() + 1, throttle);
  auto pos_brake = GkcPacketUtils::read_from_buffer(pos_steering, steering);
  GkcPacketUtils::read_from_buffer(pos_brake, brake);
}

/*
Sensors
*/
RawGkcPacket::SharedPtr SensorGkcPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(sizeof(SensorValues) + 1, 0);
  payload[0] = FIRST_BYTE;
  GkcPacketUtils::write_to_buffer(payload.begin() + 1, values);
  return std::make_unique<RawGkcPacket>(payload);
}

void SensorGkcPacket::decode(const RawGkcPacket & raw)
{
  GkcPacketUtils::read_from_buffer(
    raw.payload.begin() + 1,
    values);
}

/*
Shutdown 1
*/
RawGkcPacket::SharedPtr Shutdown1GkcPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(5, 0);
  payload[0] = FIRST_BYTE;
  GkcPacketUtils::write_to_buffer<uint32_t>(payload.begin() + 1, seq_number);
  return std::make_unique<RawGkcPacket>(payload);
}

void Shutdown1GkcPacket::decode(const RawGkcPacket & raw)
{
  GkcPacketUtils::read_from_buffer<uint32_t>(
    raw.payload.begin() + 1,
    seq_number);
}

/*
Shutdown 2
*/
RawGkcPacket::SharedPtr Shutdown2GkcPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(5, 0);
  payload[0] = FIRST_BYTE;
  GkcPacketUtils::write_to_buffer<uint32_t>(payload.begin() + 1, seq_number);
  return std::make_unique<RawGkcPacket>(payload);
}

void Shutdown2GkcPacket::decode(const RawGkcPacket & raw)
{
  GkcPacketUtils::read_from_buffer<uint32_t>(
    raw.payload.begin() + 1,
    seq_number);
}

/*
Log
*/
RawGkcPacket::SharedPtr LogPacket::encode() const
{
  GkcBuffer payload = GkcBuffer(what.size() + 1 + 1, 0);
  payload[0] = FIRST_BYTE;
  GkcPacketUtils::write_to_buffer<uint8_t>(payload.begin() + 1, static_cast<uint8_t>(level));
  std::copy(what.begin(), what.end(), payload.begin() + 2);
  return std::make_unique<RawGkcPacket>(payload);
}

void LogPacket::decode(const RawGkcPacket & raw)
{
  uint8_t level_num = 0;
  auto what_begin = GkcPacketUtils::read_from_buffer<uint8_t>(
    raw.payload.begin() + 1,
    level_num);
  level = static_cast<Severity>(level_num);
  what = std::string(what_begin, raw.payload.end());
}
}  // namespace gkc
}  // namespace tritonai
