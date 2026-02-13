#pragma once

#include <Arduino.h>

namespace MeshCore {
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
struct SELF_INFO_MSG {
  uint8_t advType;
  int8_t txPower;
  int8_t maxTxPower;
  char publicKey[32];
  int32_t advLat;
  int32_t advLon;
  uint8_t multiAck;
  uint8_t advLocPolicy;
  uint8_t telemetryMode;
  bool manualAddContacts;
  float radioFreq;
  float radioBandwidth;
  uint8_t radioSpreadingFactor;
  uint8_t radioCodingRate;
  arduino::String deviceName;
};

class Self : public SELF_INFO_MSG {
public:
  Self() {}
  void update(const SELF_INFO_MSG &info) {
    advType = info.advType;
    txPower = info.txPower;
    maxTxPower = info.maxTxPower;
    memcpy(publicKey, info.publicKey, 32);
    advLat = info.advLat;
    advLon = info.advLon;
    multiAck = info.multiAck;
    advLocPolicy = info.advLocPolicy;
    telemetryMode = info.telemetryMode;
    manualAddContacts = info.manualAddContacts;
    radioFreq = info.radioFreq;
    radioBandwidth = info.radioBandwidth;
    radioSpreadingFactor = info.radioSpreadingFactor;
    radioCodingRate = info.radioCodingRate;
    deviceName = info.deviceName;
  };
};
} // namespace MeshCore