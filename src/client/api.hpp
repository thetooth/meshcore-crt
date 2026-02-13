#pragma once

#include "channel.hpp"
#include "contact.hpp"
#include "device.hpp"
#include "packet.hpp"
#include "self.hpp"
#include "text.hpp"
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

  uint32_t readU32LE(const uint8_t *buf) {
    return static_cast<uint32_t>(buf[0]) | (static_cast<uint32_t>(buf[1]) << 8) |
           (static_cast<uint32_t>(buf[2]) << 16) | (static_cast<uint32_t>(buf[3]) << 24);
  };
  int32_t readS32LE(const uint8_t *buf) {
    return static_cast<int32_t>(buf[0]) | (static_cast<int32_t>(buf[1]) << 8) |
           (static_cast<int32_t>(buf[2]) << 16) | (static_cast<int32_t>(buf[3]) << 24);
  };
  void writeU32LE(uint8_t *buf, uint32_t value) {
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    buf[2] = (value >> 16) & 0xFF;
    buf[3] = (value >> 24) & 0xFF;
  };
  void writeS32LE(uint8_t *buf, int32_t value) {
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    buf[2] = (value >> 16) & 0xFF;
    buf[3] = (value >> 24) & 0xFF;
  };

  // An inbound frame starts with byte 60 (ASCII '<'), then 2 bytes with frame length, followed by actual
  // frame.
  void send(const char *data, uint16_t len) {
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

  void requestChannels() { channelRequestStart = true; }

  void requestContacts() {
    auto cmd = CMD_GET_CONTACTS;
    writeU32LE((uint8_t *)cmd + 1, contacts.since);
    send(cmd, sizeof(cmd));
  }

  void requestNextMessage() {
    char REQ_NEXT[] = { 0x0A };
    send(REQ_NEXT, sizeof(REQ_NEXT));
  }

  void sendTextMessage(const arduino::String &text, uint8_t type = 0, uint8_t attempts = 0) {
    if (toRecipient.length() <= 0) {
      console.print("No recipient specified.");
      console.print("Use /to <name> to specify recipient.");
      return;
    }

    char buf[256] = { 0 };
    buf[0] = CMD_SEND_TXT_MSG[0];
    buf[1] = type;
    buf[2] = attempts;
    writeU32LE((uint8_t *)buf + 3, millis());
    memcpy(buf + 7, toRecipient.c_str(), 6);
    memcpy(buf + 13, text.c_str(), text.length());

    send(buf, 13 + text.length());
  }

  void handleContactMsg(const CONTACT_MGS &msg) {
    auto pubKeyPrefix = arduino::String((char *)msg.pubKeyPrefix, 6);

    arduino::String name = "UNKNOWN";
    if (contacts.contains(pubKeyPrefix)) {
      auto &c = contacts[pubKeyPrefix];
      name = c.advName;
      c.messages.push(msg.msg);
    }

    console.print("DIRECT " + name + ": ");
    console.print(String(msg.msg));
    activity = true;
  }

  void handleChannelMsg(const CHANNEL_MSG &msg) {
    auto &ch = channels.at(msg.channelIndex);
    ch.messages.push(msg.msg);

    console.print("CHANNEL " + String(ch.name) + ": ");
    console.print(String(msg.msg));
    activity = true;
  }

  void handlePacket(uint8_t *data, uint16_t len) {
    uint8_t offset = 0;
    uint8_t type = data[offset++];

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
      contact.advName = Text::sanitizeUtf8ToAscii(data + offset, 32, true);
      offset += 32;
      contact.lastAdvert = readU32LE(data + offset);
      offset += 4;
      contact.advLat = readS32LE(data + offset);
      offset += 4;
      contact.advLon = readS32LE(data + offset);
      offset += 4;
      contact.lastMod = readU32LE(data + offset);
      offset += 4;
      contact.lastMod = readU32LE(data + offset);
      offset += 4;

      // Use the first 6 bytes of the public key as a key in our contacts map, since the device only sends the
      // prefix for messages
      auto pubKeyPrefix = arduino::String((char *)contact.pubKey, 6);

      auto &c = contacts[pubKeyPrefix];

      c.update(contact);
    } break;
    case PACKET_CONTACT_END:
      if (len > 1) {
        contacts.since = readU32LE(data + offset);
        offset += 4;
      }

      // console.print("Contacts: " + String(contacts.count));
      activity = true;

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
      payload.name = Text::sanitizeUtf8ToAscii(data + offset, 32, true);
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

      auto &ch = channels[payload.channelIndex];

      ch.update(payload);
      channels.lastSyncOK = true;
    } break;
    case PACKET_SELF_INFO: {
      auto payload = SELF_INFO_MSG{};

      payload.advType = data[offset++];
      payload.txPower = data[offset++];
      payload.maxTxPower = data[offset++];
      memcpy(payload.publicKey, data + offset, 32);
      offset += 32;
      payload.advLat = readS32LE(data + offset);
      offset += 4;
      payload.advLon = readS32LE(data + offset);
      offset += 4;
      payload.multiAck = data[offset++];
      payload.advLocPolicy = data[offset++];
      payload.telemetryMode = data[offset++];
      payload.manualAddContacts = data[offset++] != 0;
      payload.radioFreq = readU32LE(data + offset) / 1000.0;
      offset += 4;
      payload.radioBandwidth = readU32LE(data + offset) / 1000.0;
      offset += 4;
      payload.radioSpreadingFactor = data[offset++];
      payload.radioCodingRate = data[offset++];
      payload.deviceName = Text::sanitizeUtf8ToAscii(data + offset, 32, true);
      offset += 32;

      self.update(payload);
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

      payload.msg = Text::sanitizeUtf8ToAscii(data + offset, len - offset);

      handleContactMsg(payload);
    } break;
    case PACKET_CHANNEL_MSG_RECV: {
      auto payload = CHANNEL_MSG{};

      payload.channelIndex = data[offset++];
      payload.pathLen = data[offset++];
      payload.type = data[offset++];
      payload.timestamp = readU32LE(data + offset);
      offset += 4;

      payload.msg = Text::sanitizeUtf8ToAscii(data + offset, len - offset);

      handleChannelMsg(payload);
    } break;
    case PACKET_ADVERTISEMENT: {
      Serial.println("Received advertisement packet");
      requestContacts();
    } break;
    }

    if (channelRequestStart) {
      if (channels.requestAll([this](uint8_t channelIndex) { requestChannelInfo(channelIndex); })) {
        channelRequestStart = false;
      }
    }
  }

  bool activity = false;
  bool msgWaiting = true;

  bool channelRequestStart = false;

  arduino::String toRecipient;

  Device device;
  Self self;
  ChannelList channels;
  ContactList contacts;

private:
  UI::Console &console;
};
} // namespace MeshCore