#pragma once

#include "api.hpp"

namespace MeshCore {
class SerialInterface {
public:
  SerialInterface(Client &client) : client(client) {}

  enum class RadioRxState : uint8_t { WaitStart, ReadLenLow, ReadLenHigh, ReadFrame };

  RadioRxState radioState = RadioRxState::WaitStart;
  uint8_t frameDir = 0;
  uint16_t frameLength = 0;
  uint16_t frameIndex = 0;
  uint8_t frameBuffer[MAX_TRANS_UNIT];

  Client &client;

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
          Serial.println("Bad frame length: " + String(frameLength));
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

          Serial.print("FRAME: ");
          for (int i = 0; i < frameLength; i++) {
            Serial.print(frameBuffer[i], HEX);
            Serial.print(" ");
          }
          Serial.println();

          client.handlePacket(frameBuffer, frameLength);

          radioState = RadioRxState::WaitStart;
        }
        break;
      }
    }
  };
};
} // namespace MeshCore