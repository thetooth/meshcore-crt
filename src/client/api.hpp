#pragma once

#include "meshcore/Packet.h"
#include "ui/console.hpp"

#include <Arduino.h>
#include <stdint.h>

namespace {
enum class RadioRxState : uint8_t { WaitStart, ReadLenLow, ReadLenHigh, ReadFrame };

RadioRxState radioState = RadioRxState::WaitStart;
uint8_t frameDir = 0;
uint16_t frameLength = 0;
uint16_t frameIndex = 0;
uint8_t frameBuffer[MAX_TRANS_UNIT];

class MeshClient {
public:
  MeshClient(UI::Console &console) : console(console) {}

  void receiveRadio() {
    while (Serial1.available()) {
      uint8_t b = (uint8_t)Serial1.read();

      switch (radioState) {
      case RadioRxState::WaitStart:
        if (b == '>' || b == '<') {
          frameDir = b;
          radioState = RadioRxState::ReadLenLow;
        }
        break;
      case RadioRxState::ReadLenLow:
        frameLength = b;
        radioState = RadioRxState::ReadLenHigh;
        break;
      case RadioRxState::ReadLenHigh:
        frameLength |= (uint16_t)b << 8;
        if (frameLength == 0 || frameLength > sizeof(frameBuffer)) {
          // Bad length, reset and wait for next frame start.
          radioState = RadioRxState::WaitStart;
          break;
        }
        frameIndex = 0;
        radioState = RadioRxState::ReadFrame;
        break;
      case RadioRxState::ReadFrame:
        frameBuffer[frameIndex++] = b;
        if (frameIndex >= frameLength) {
          if (frameDir == '>') {
            Serial.println("UART OUT");
          } else if (frameDir == '<') {
            Serial.println("UART IN");
          }

          Serial.println(" LEN " + String(frameLength));

          mesh::Packet p;
          if (p.readFrom(frameBuffer, (uint8_t)frameLength)) {
            // TODO
          } else {
            Serial.println("BAD PACKET");
          }

          Serial.println(" TYPE " + String(p.getPayloadType()));
          Serial.println(" VER " + String(p.getPayloadVer()));
          Serial.println(" ROUTE " + String(p.getRouteType()));
          Serial.println(" SNR: " + String(p.getSNR()) + "dB");

          radioState = RadioRxState::WaitStart;
        }
        break;
      }
    }
  };

  UI::Console &console;
};
} // namespace