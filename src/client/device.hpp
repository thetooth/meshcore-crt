#pragma once

#include <Arduino.h>

namespace MeshCore {
// Byte 0: 0x0D
// Byte 1: Firmware Version (uint8)
// Bytes 2+: Variable length based on firmware version

// For firmware version >= 3:
// Byte 2: Max Contacts Raw (uint8, actual = value * 2)
// Byte 3: Max Channels (uint8)
// Bytes 4-7: BLE PIN (32-bit little-endian)
// Bytes 8-19: Firmware Build (12 bytes, UTF-8, null-padded)
// Bytes 20-59: Model (40 bytes, UTF-8, null-padded)
// Bytes 60-79: Version (20 bytes, UTF-8, null-padded)
struct DEVICE_INFO_MSG {
  uint8_t firmwareVersion;
  uint8_t maxContacts;
  uint8_t maxChannels;
  uint32_t blePin;
  char firmwareBuild[12];
  char manufacturerModel[40];
  char semVersion[20];
};

class Device : public DEVICE_INFO_MSG {
public:
  Device() {}
  void update(const DEVICE_INFO_MSG &info) {
    firmwareVersion = info.firmwareVersion;
    maxContacts = info.maxContacts;
    maxChannels = info.maxChannels;
    blePin = info.blePin;
    memcpy(firmwareBuild, info.firmwareBuild, 12);
    memcpy(manufacturerModel, info.manufacturerModel, 40);
    memcpy(semVersion, info.semVersion, 20);
  };
};
} // namespace MeshCore