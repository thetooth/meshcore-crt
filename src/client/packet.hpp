#pragma once

#include <stdint.h>

#define MAX_PACKET_PAYLOAD 184
#define MAX_PATH_SIZE      64
#define MAX_TRANS_UNIT     255

namespace MeshCore {

// Byte 0: 0x01
// Byte 1: 0x03
// Bytes 2-10: "mccli" (ASCII, null-padded to 9 bytes)
char CMD_APP_START[] = { 0x01, 0x03, 'm', 'c', 'c', 'l', 'i', 0, 0, 0 };

// Byte 0: 0x1F
// Byte 1: Channel Index (0-7)
char CMD_CHANNEL_INFO[] = { 0x1F, 0x00 };

// Byte 0: 0x04
// Byte 1-4: Optional value (32-bit little-endian integer)
char CMD_GET_CONTACTS[] = { 0x04, 0x00, 0x00, 0x00, 0x00 };

// Byte 0: 0x02
// Byte 1: Text Type (0 = plain)
// Byte 2: Attempt (0-3)
// Bytes 3-6: Sender Timestamp (32-bit little-endian)
// Bytes 7-12: Public Key Prefix (6 bytes)
// Bytes 13+: Message Text (UTF-8, max length: 160 bytes)
char CMD_SEND_TXT_MSG[] = { 0x02 };

// Byte 0: 0x00
// Bytes 1-4: Optional value (32-bit little-endian integer)
constexpr uint8_t PACKET_OK = 0x00;
// Byte 0: 0x01
// Byte 1: Error code (optional)
constexpr uint8_t PACKET_ERR = 0x01;
constexpr uint8_t PACKET_CONTACT_START = 0x02;
constexpr uint8_t PACKET_CONTACT = 0x03;
constexpr uint8_t PACKET_CONTACT_END = 0x04;
// Byte 0: 0x05
// Byte 1: Advertisement Type
// Byte 2: TX Power
// Byte 3: Max TX Power
// Bytes 4-35: Public Key (32 bytes, hex)
// Bytes 36-39: Advertisement Latitude (32-bit little-endian, divided by 1e6)
// Bytes 40-43: Advertisement Longitude (32-bit little-endian, divided by 1e6)
// Byte 44: Multi ACKs
// Byte 45: Advertisement Location Policy
// Byte 46: Telemetry Mode (bitfield)
// Byte 47: Manual Add Contacts (bool)
// Bytes 48-51: Radio Frequency (32-bit little-endian, divided by 1000.0)
// Bytes 52-55: Radio Bandwidth (32-bit little-endian, divided by 1000.0)
// Byte 56: Radio Spreading Factor
// Byte 57: Radio Coding Rate
// Bytes 58+: Device Name (UTF-8, variable length, null-terminated)
constexpr uint8_t PACKET_SELF_INFO = 0x05;
// Byte 0: 0x06
// Byte 1: Message Type
// Bytes 2-5: Expected ACK (4 bytes, hex)
// Bytes 6-9: Suggested Timeout (32-bit little-endian, seconds)
constexpr uint8_t PACKET_MSG_SENT = 0x06;
// Byte 0: 0x07 (packet type)
// Bytes 1-6: Public Key Prefix (6 bytes, hex)
// Byte 7: Path Length
// Byte 8: Text Type
// Bytes 9-12: Timestamp (32-bit little-endian)
// Bytes 13-16: Signature (4 bytes, only if txt_type == 2)
// Bytes 17+: Message Text (UTF-8)
constexpr uint8_t PACKET_CONTACT_MSG_RECV = 0x07;
// Byte 0: 0x08 (packet type)
// Byte 1: Channel Index (0-7)
// Byte 2: Path Length
// Byte 3: Text Type
// Bytes 4-7: Timestamp (32-bit little-endian)
// Bytes 8+: Message Text (UTF-8)
constexpr uint8_t PACKET_CHANNEL_MSG_RECV = 0x08;
constexpr uint8_t PACKET_CURRENT_TIME = 0x09;
// Byte 0: 0x0A (packet type)
constexpr uint8_t PACKET_NO_MORE_MSGS = 0x0A;
// Byte 0: 0x0C
// Bytes 1-2: Battery Level (16-bit little-endian, percentage 0-100)

// Optional (if data size > 3):
// Bytes 3-6: Used Storage (32-bit little-endian, KB)
// Bytes 7-10: Total Storage (32-bit little-endian, KB)
constexpr uint8_t PACKET_BATTERY = 0x0B;
// For firmware version >= 3:
// Byte 2: Max Contacts Raw (uint8, actual = value * 2)
// Byte 3: Max Channels (uint8)
// Bytes 4-7: BLE PIN (32-bit little-endian)
// Bytes 8-19: Firmware Build (12 bytes, UTF-8, null-padded)
// Bytes 20-59: Model (40 bytes, UTF-8, null-padded)
// Bytes 60-79: Version (20 bytes, UTF-8, null-padded)
constexpr uint8_t PACKET_DEVICE_INFO = 0x0D;
// Byte 0: 0x10 (packet type)
// Byte 1: SNR (signed byte, multiplied by 4)
// Bytes 2-3: Reserved
// Bytes 4-9: Public Key Prefix (6 bytes, hex)
// Byte 10: Path Length
// Byte 11: Text Type
// Bytes 12-15: Timestamp (32-bit little-endian)
// Bytes 16-19: Signature (4 bytes, only if txt_type == 2)
// Bytes 20+: Message Text (UTF-8)
constexpr uint8_t PACKET_CONTACT_MSG_RECV_V3 = 0x10;
// Byte 0: 0x11 (packet type)
// Byte 1: SNR (signed byte, multiplied by 4)
// Bytes 2-3: Reserved
// Byte 4: Channel Index (0-7)
// Byte 5: Path Length
// Byte 6: Text Type
// Bytes 7-10: Timestamp (32-bit little-endian)
// Bytes 11+: Message Text (UTF-8)
constexpr uint8_t PACKET_CHANNEL_MSG_RECV_V3 = 0x11;
// Byte 0: 0x12
// Byte 1: Channel Index
// Bytes 2-33: Channel Name (32 bytes, null-terminated)
// Bytes 34-65: Secret (32 bytes, but device typically only returns 20 bytes total)
constexpr uint8_t PACKET_CHANNEL_INFO = 0x12;
constexpr uint8_t PACKET_ADVERTISEMENT = 0x80;
// Byte 0: 0x82
// Bytes 1-6: ACK Code (6 bytes, hex)
constexpr uint8_t PACKET_ACK = 0x82;
// Byte 0: 0x83 (packet type)
constexpr uint8_t PACKET_MESSAGES_WAITING = 0x83;
constexpr uint8_t PACKET_LOG_DATA = 0x88;
}; // namespace MeshCore