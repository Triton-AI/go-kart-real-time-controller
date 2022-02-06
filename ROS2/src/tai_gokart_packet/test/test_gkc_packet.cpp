/**
 * @file test_gkc_packet.cpp
 * @author Haoru Xue (hxue@ucsd.edu)
 * @brief
 * @version 0.1
 * @date 2021-11-03
 *
 * @copyright Copyright 2021 Triton AI
 *
 */

#include "gtest/gtest.h"

#include "tai_gokart_packet/gkc_packets.hpp"
#include "tai_gokart_packet/gkc_packet_factory.hpp"
class Sub : public tritonai::gkc::GkcPacketSubscriber
{
public:
  void packet_callback(const tritonai::gkc::Handshake1GkcPacket & packet) {(void)packet;}
  void packet_callback(const tritonai::gkc::Handshake2GkcPacket & packet) {(void)packet;}
  void packet_callback(const tritonai::gkc::GetFirmwareVersionGkcPacket & packet) {(void)packet;}
  void packet_callback(const tritonai::gkc::FirmwareVersionGkcPacket & packet) {(void)packet;}
  void packet_callback(const tritonai::gkc::ResetMcuGkcPacket & packet) {(void)packet;}
  void packet_callback(const tritonai::gkc::HeartbeatGkcPacket & packet) {(void)packet;}
  void packet_callback(const tritonai::gkc::ConfigGkcPacket & packet) {(void)packet;}
  void packet_callback(const tritonai::gkc::StateTransitionGkcPacket & packet) {(void)packet;}
  void packet_callback(const tritonai::gkc::ControlGkcPacket & packet) {(void)packet;}
  void packet_callback(const tritonai::gkc::SensorGkcPacket & packet) {(void)packet;}
  void packet_callback(const tritonai::gkc::Shutdown1GkcPacket & packet) {(void)packet;}
  void packet_callback(const tritonai::gkc::Shutdown2GkcPacket & packet) {(void)packet;}
  void packet_callback(const tritonai::gkc::LogPacket & packet) {(void)packet;}
};

TEST(TestGkcPacketUtils, CreatePacket) {
  auto packet = tritonai::gkc::GkcPacketUtils::CreatePacket<tritonai::gkc::Handshake1GkcPacket>();
  SUCCEED();
}

TEST(TestGkcPacketUtils, BufferReadWrite) {
  tritonai::gkc::GkcBuffer buffer(4, 0);
  auto buffer_end =
    tritonai::gkc::GkcPacketUtils::write_to_buffer(buffer.begin(), static_cast<int32_t>(1234));
  EXPECT_EQ(buffer_end, buffer.end());
  int32_t val = 0;
  tritonai::gkc::GkcPacketUtils::read_from_buffer(buffer.begin(), val);
  EXPECT_EQ(val, 1234);
  SUCCEED();
}

TEST(TestGkcPackets, Handshake1GkcPacket) {
  auto packet = tritonai::gkc::Handshake1GkcPacket();
  packet.seq_number = 0xABCD1234;
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::Handshake1GkcPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::Handshake1GkcPacket();
  reconstructed_packet.decode(*raw_packet);
  EXPECT_EQ(reconstructed_packet.seq_number, packet.seq_number);
  SUCCEED();
}

TEST(TestGkcPackets, Handshake2GkcPacket) {
  auto packet = tritonai::gkc::Handshake2GkcPacket();
  packet.seq_number = 0xABCD1234;
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::Handshake2GkcPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::Handshake2GkcPacket();
  reconstructed_packet.decode(*raw_packet);
  EXPECT_EQ(reconstructed_packet.seq_number, packet.seq_number);
  SUCCEED();
}

TEST(TestGkcPackets, GetFirmwareVersionGkcPacket) {
  auto packet = tritonai::gkc::GetFirmwareVersionGkcPacket();
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::GetFirmwareVersionGkcPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::GetFirmwareVersionGkcPacket();
  reconstructed_packet.decode(*raw_packet);
  SUCCEED();
}

TEST(TestGkcPackets, FirmwareVersionGkcPacket) {
  auto packet = tritonai::gkc::FirmwareVersionGkcPacket();
  packet.major = static_cast<uint8_t>(22);
  packet.minor = static_cast<uint8_t>(23);
  packet.patch = static_cast<uint8_t>(24);
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::FirmwareVersionGkcPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::FirmwareVersionGkcPacket();
  reconstructed_packet.decode(*raw_packet);
  EXPECT_EQ(reconstructed_packet.major, packet.major);
  EXPECT_EQ(reconstructed_packet.minor, packet.minor);
  EXPECT_EQ(reconstructed_packet.patch, packet.patch);
  SUCCEED();
}

TEST(TestGkcPackets, ResetMcuGkcPacket) {
  auto packet = tritonai::gkc::ResetMcuGkcPacket();
  packet.magic_number = 0xABCD1234;
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::ResetMcuGkcPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::ResetMcuGkcPacket();
  reconstructed_packet.decode(*raw_packet);
  EXPECT_EQ(reconstructed_packet.magic_number, packet.magic_number);
  SUCCEED();
}

TEST(TestGkcPackets, HeartbeatGkcPacket) {
  auto packet = tritonai::gkc::HeartbeatGkcPacket();
  packet.rolling_counter = static_cast<uint8_t>(22);
  packet.state = static_cast<uint8_t>(12);
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::HeartbeatGkcPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::HeartbeatGkcPacket();
  reconstructed_packet.decode(*raw_packet);
  EXPECT_EQ(reconstructed_packet.rolling_counter, packet.rolling_counter);
  EXPECT_EQ(reconstructed_packet.state, packet.state);
  SUCCEED();
}

TEST(TestGkcPackets, ConfigGkcPacket) {
  auto packet = tritonai::gkc::ConfigGkcPacket();
  packet.values.max_steering_left = 1.4321;
  packet.values.neutral_steering = 0.1234;
  packet.values.control_timeout_ms = static_cast<uint32_t>(123);
  packet.values.sensor_timeout_ms = static_cast<uint32_t>(321);
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::ConfigGkcPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::ConfigGkcPacket();
  reconstructed_packet.decode(*raw_packet);
  EXPECT_EQ(reconstructed_packet.values.max_steering_left, packet.values.max_steering_left);
  EXPECT_EQ(reconstructed_packet.values.neutral_steering, packet.values.neutral_steering);
  EXPECT_EQ(reconstructed_packet.values.control_timeout_ms, packet.values.control_timeout_ms);
  EXPECT_EQ(reconstructed_packet.values.sensor_timeout_ms, packet.values.sensor_timeout_ms);
  SUCCEED();
}

TEST(TestGkcPackets, StateTransitionGkcPacket) {
  auto packet = tritonai::gkc::StateTransitionGkcPacket();
  packet.requested_state = static_cast<uint8_t>(22);
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::StateTransitionGkcPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::StateTransitionGkcPacket();
  reconstructed_packet.decode(*raw_packet);
  EXPECT_EQ(reconstructed_packet.requested_state, packet.requested_state);
  SUCCEED();
}

TEST(TestGkcPackets, ControlGkcPacket) {
  auto packet = tritonai::gkc::ControlGkcPacket();
  packet.throttle = 0.555;
  packet.steering = 0.4321;
  packet.brake = 1234.0;
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::ControlGkcPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::ControlGkcPacket();
  reconstructed_packet.decode(*raw_packet);
  EXPECT_EQ(reconstructed_packet.throttle, packet.throttle);
  EXPECT_EQ(reconstructed_packet.steering, packet.steering);
  EXPECT_EQ(reconstructed_packet.brake, packet.brake);
  SUCCEED();
}

TEST(TestGkcPackets, SensorGkcPacket) {
  auto packet = tritonai::gkc::SensorGkcPacket();
  packet.values.wheel_speed_fl = 513.0;
  packet.values.brake_pressure = 1234.0;
  packet.values.fault_steering = false;
  packet.values.fault_info = true;
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::SensorGkcPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::SensorGkcPacket();
  reconstructed_packet.decode(*raw_packet);
  EXPECT_EQ(reconstructed_packet.values.wheel_speed_fl, packet.values.wheel_speed_fl);
  EXPECT_EQ(reconstructed_packet.values.brake_pressure, packet.values.brake_pressure);
  EXPECT_EQ(reconstructed_packet.values.fault_steering, packet.values.fault_steering);
  EXPECT_EQ(reconstructed_packet.values.fault_info, packet.values.fault_info);
  SUCCEED();
}

TEST(TestGkcPackets, Shutdown1GkcPacket) {
  auto packet = tritonai::gkc::Shutdown1GkcPacket();
  packet.seq_number = 0xABCD1234;
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::Shutdown1GkcPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::Shutdown1GkcPacket();
  reconstructed_packet.decode(*raw_packet);
  EXPECT_EQ(reconstructed_packet.seq_number, packet.seq_number);
  SUCCEED();
}

TEST(TestGkcPackets, Shutdown2GkcPacket) {
  auto packet = tritonai::gkc::Shutdown2GkcPacket();
  packet.seq_number = 0xABCD1234;
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::Shutdown2GkcPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::Shutdown2GkcPacket();
  reconstructed_packet.decode(*raw_packet);
  EXPECT_EQ(reconstructed_packet.seq_number, packet.seq_number);
  SUCCEED();
}

TEST(TestGkcPackets, LogPacket) {
  auto packet = tritonai::gkc::LogPacket();
  packet.level = tritonai::gkc::LogPacket::Severity::CRITICAL;
  packet.what = "Hello World";
  auto raw_packet = packet.encode();
  EXPECT_EQ(raw_packet->payload[0], tritonai::gkc::LogPacket::FIRST_BYTE);
  auto reconstructed_packet = tritonai::gkc::LogPacket();
  reconstructed_packet.decode(*raw_packet);
  EXPECT_EQ(reconstructed_packet.level, packet.level);
  EXPECT_EQ(reconstructed_packet.what, packet.what);
  SUCCEED();
}
