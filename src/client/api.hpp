#pragma once

#include "packet.hpp"
#include "ui/console.hpp"

#include <Arduino.h>
#include <stdint.h>

namespace MeshCore {

struct CONTACT_MGS {
  uint8_t pubKeyPrefix[6];
  uint8_t pathLen;
  uint8_t type;
  uint32_t timestamp;
  arduino::String msg;
};

struct CHANNEL_MSG {
  uint8_t channelIndex;
  uint8_t pathLen;
  uint8_t type;
  uint32_t timestamp;
  arduino::String msg;
};

class Client {
public:
  Client(UI::Console &console) : console(console) {}

  // An inbound frame starts with byte 60 (ASCII '<'), then 2 bytes with frame length, followed by actual
  // frame.
  void send(const char *data, uint16_t len) {
    Serial.print("Sending frame: ");
    Serial.print('<', HEX);
    Serial.print(len, HEX);
    Serial.print(len >> 8, HEX);
    for (int i = 0; i < len; i++) {
      Serial.print((uint8_t)data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    Serial1.write("<");
    Serial1.write(len);
    Serial1.write(len >> 8);
    Serial1.write(data, len);
  }

  void appStart() { send(CMD_APP_START, sizeof(CMD_APP_START)); }

  void requestNextMessage() {
    char REQ_NEXT[] = { 0x0A };
    send(REQ_NEXT, sizeof(REQ_NEXT));
  }

  void handleContactMsg(const CONTACT_MGS &msg) {
    console.print("CONTACT_MSG from " + String((char *)msg.pubKeyPrefix) + ": " + String(msg.msg));
  }

  void handleChannelMsg(const CHANNEL_MSG &msg) {
    console.print("CHANNEL_MSG to " + String(msg.channelIndex) + ": " + String(msg.msg));
  }

  void handlePacket(uint8_t *data, uint16_t len) {
    uint8_t offset = 0;
    uint8_t type = data[offset++];
    Serial.print("Packet type: ");
    Serial.println(type, HEX);

    switch (type) {
    case PACKET_MESSAGES_WAITING:
      Serial.println("Messages waiting!");
      msgWaiting = true;
      break;
    case PACKET_NO_MORE_MSGS:
      Serial.println("No more messages.");
      msgWaiting = false;
      break;
    case PACKET_CONTACT_MSG_RECV: {
      auto payload = CONTACT_MGS{};

      memcpy(payload.pubKeyPrefix, data + offset, 6);
      offset += 6;
      payload.pathLen = data[offset++];
      payload.type = data[offset++];

      payload.timestamp =
          (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
      offset += 4;

      if (payload.type == 2) {
        offset += 4;
      }

      payload.msg = arduino::String(data + offset, len - offset);

      handleContactMsg(payload);
    } break;
    case PACKET_CHANNEL_MSG_RECV: {
      auto payload = CHANNEL_MSG{};

      payload.channelIndex = data[offset++];
      payload.pathLen = data[offset++];
      payload.type = data[offset++];
      payload.timestamp =
          (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
      offset += 4;

      payload.msg = arduino::String(data + offset, len - offset);

      handleChannelMsg(payload);
      break;
    }
    }
  }

  UI::Console &console;
  bool msgWaiting = true;
};
} // namespace MeshCore