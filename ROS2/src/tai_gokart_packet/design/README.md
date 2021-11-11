# Gokart Packet Library

## Introduction

This packet library includes

- `gkc_packets.hpp`: class definition of all packets;
- `gkc_packet_factory.hpp`: factory class for encoding and decoding packets;
- `gkc_packet_subscriber.hpp`: interface for receiving packets from the packet factory.

Note:

1. `gkc` means "gokart controller";
2. All packets are subclasses of `GkcPacket`;

## How to use

Suppose you are writing a microcontroller code that uses this library to communicate with the PC.

1. Have two of your classes ready: one presumably called `message_handler` that takes care of the inbound/outbound messages on the high level. The other one maybe called `comm_handler` that directly reads/writes bytes onto some communication lines.
2. Your `message_handler` needs to subclass `GkcPacketSubscriber` and implements all of its interface methods. They are the different callbacks when the factory receives every message.
3. Declare an instance of `GkcPacketFactory` by passing your `message_handler` and a debug callback (maybe `&GkcPacketUtils::debug_cout`) to the constructor. Your `GkcPacketFactory` could live inside your `message_handler`, for example.
4. Your `comm_handler` should convert incoming bytes into a `std::vector<uint8_t>` which is typedef-ed as `GkcBuffer` should you include `gkc_packets.hpp`. Then it should call `GkcPacketFactory::Receive` and pass the buffer to the factory. The factory will parse the bytes and trigger the callbacks to your `message_handler`.
5. When your `message_handler` whats to send out messages, It should call `GkcPacketFactory::Send` and be handed back with a `GkcBuffer` ready to be sent down the communication line using your `comm_handler`.

Note:

1. Make sure that the `GkcBuffer` passed to `GkcPacketFactory::Receive` only includes valid data stream. For example, Do not initialize a vector of size 256, dump the 42 bytes received into it, and call the callback - this will corrupt the packets, and some low-level communication libraries do this.
2. Preallocate space when initializing `GkcBuffer`. For example:
```cpp
#include <memory>
#include "tai_gokart_packet/gkc_packets.hpp"

// your incoming buffer
unsigned char* some_buffer = ...
// length of the buffer
int len_buffer = 16;

// pre-allocate enough space
GkcBuffer gkc_buffer(len_buffer);
// fast copy
std::copy(some_buffer, some_buffer + len_buffer, gkc_buffer.begin());

// Pass the buffer to the factory. 
// Internally the pre-allocation also makes decoding faster
packet_factory.receive(gkc_buffer);
```