#pragma once

#include "channel.hpp"
#include "contact.hpp"
#include "packet.hpp"
#include "ui/console.hpp"

#include <Arduino.h>
#include <array>
#include <map>
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

  void requestChannelInfo(uint8_t channelIndex) {
    char cmd[2] = { CMD_CHANNEL_INFO[0], channelIndex };
    send(cmd, sizeof(cmd));
  }

  bool requestAllChannels() {
    return channels.requestAll([this](uint8_t channelIndex) { requestChannelInfo(channelIndex); });
  }

  void requestContacts() { send(CMD_GET_CONTACTS, sizeof(CMD_GET_CONTACTS)); }

  void requestNextMessage() {
    char REQ_NEXT[] = { 0x0A };
    send(REQ_NEXT, sizeof(REQ_NEXT));
  }

  void handleContactMsg(const CONTACT_MGS &msg) {
    auto pubKeyPrefix = arduino::String((char *)msg.pubKeyPrefix, 6);

    arduino::String name = "UNKNOWN";
    if (auto it = contacts.find(pubKeyPrefix); it != contacts.end()) {
      name = it->second.advName;
    }

    console.print("CONTACT_MSG from " + name + ": ");
    console.print(String(msg.msg));
  }

  void handleChannelMsg(const CHANNEL_MSG &msg) {
    auto &ch = channels.at(msg.channelIndex);
    console.print("CHANNEL_MSG to " + String(ch.name) + ": ");
    console.print(String(msg.msg));
  }

  void handlePacket(uint8_t *data, uint16_t len) {
    uint8_t offset = 0;
    uint8_t type = data[offset++];
    auto readU32LE = [](const uint8_t *buf) -> uint32_t {
      return static_cast<uint32_t>(buf[0]) | (static_cast<uint32_t>(buf[1]) << 8) |
             (static_cast<uint32_t>(buf[2]) << 16) | (static_cast<uint32_t>(buf[3]) << 24);
    };
    auto read32LE = [](const uint8_t *buf) -> int32_t {
      return static_cast<int32_t>(buf[0]) | (static_cast<int32_t>(buf[1]) << 8) |
             (static_cast<int32_t>(buf[2]) << 16) | (static_cast<int32_t>(buf[3]) << 24);
    };
    // Serial.print("Packet type: ");
    // Serial.println(type, HEX);

    switch (type) {
    case PACKET_CONTACT_START:
      contacts.clear();
      contacts.count = readU32LE(data + offset);
      offset += 4;

      Serial.println("Contact count: " + String(contacts.count));
      break;
    case PACKET_CONTACT: {
      auto contact = CONTACT_INFO_MSG{};
      memcpy(contact.pubKey, data + offset, 32);
      offset += 32;
      contact.type = data[offset++];
      contact.flags = data[offset++];
      contact.outPathLen = data[offset++];
      memcpy(contact.outPath, data + offset, 64);
      offset += 64;
      contact.advName = arduino::String((char *)data + offset, strnlen((char *)data + offset, 32));
      offset += 32;
      contact.lastAdvert = readU32LE(data + offset);
      offset += 4;
      contact.advLat = read32LE(data + offset);
      offset += 4;
      contact.advLon = read32LE(data + offset);
      offset += 4;
      contact.lastMod = readU32LE(data + offset);
      offset += 4;
      contact.lastMod = readU32LE(data + offset);
      offset += 4;

      // Use the first 6 bytes of the public key as a key in our contacts map, since the device only sends the
      // prefix for messages
      auto pubKeyPrefix = arduino::String((char *)contact.pubKey, 6);

      Serial.println("Received contact: " + contact.advName + " with pubKeyPrefix: ");
      for (int i = 0; i < 6; i++) {
        Serial.print((uint8_t)contact.pubKey[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      Serial.println("Adv: " + String(contact.advLat) + ", " + String(contact.advLon));
      Serial.println("Last Advert: " + String(contact.lastAdvert));
      Serial.println("Last Mod: " + String(contact.lastMod));

      contacts[pubKeyPrefix] = Contact{ contact };
    } break;
    case PACKET_CONTACT_END:
      if (len > 1) {
        contacts.since = readU32LE(data + offset);
        offset += 4;
      }

      console.print("Contacts: " + String(contacts.count));

      break;
    case PACKET_MESSAGES_WAITING:
      Serial.println("Messages waiting!");
      msgWaiting = true;
      break;
    case PACKET_NO_MORE_MSGS:
      Serial.println("No more messages.");
      msgWaiting = false;
      break;
    case PACKET_CHANNEL_INFO: {
      auto payload = CHANNEL_INFO_MSG{};

      payload.channelIndex = data[offset++];
      payload.name = arduino::String((char *)data + offset, strnlen((char *)data + offset, 32));
      offset += 32;

      // Secret is typically 20 bytes, but device returns 32 bytes total for name+secret, so we need to
      // calculate actual secret length
      uint8_t secretLen = len - offset;
      if (secretLen > 32) {
        secretLen = 32;
      }
      uint8_t secret[32];
      memcpy(secret, data + offset, secretLen);
      offset += secretLen;

      channels.at(payload.channelIndex) = Channel{ payload };
      channels.lastSyncOK = true;
      // console.print("CHANNEL INFO: " + String(payload.channelIndex) + " - " + payload.name);
    } break;
    case PACKET_SELF_INFO: {
      Serial.println("Received self info packet");
    } break;
    case PACKET_CONTACT_MSG_RECV: {
      auto payload = CONTACT_MGS{};

      memcpy(payload.pubKeyPrefix, data + offset, 6);
      offset += 6;
      payload.pathLen = data[offset++];
      payload.type = data[offset++];

      payload.timestamp = readU32LE(data + offset);
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
      payload.timestamp = readU32LE(data + offset);
      offset += 4;

      payload.msg = arduino::String(data + offset, len - offset);

      handleChannelMsg(payload);
      break;
    }
    }
  }

  bool msgWaiting = true;

  ChannelList channels;
  ContactList contacts;
  // std::array<Contact, 350> contacts;

private:
  UI::Console &console;
};
} // namespace MeshCore