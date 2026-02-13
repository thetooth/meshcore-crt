#pragma once

#include "../client/api.hpp"
#include "console.hpp"

namespace UI {
class Prompt {
public:
  Prompt(MeshCore::Client &client, UI::Console &console, bool &sleep)
      : client(client), console(console), sleep(sleep) {}

  arduino::String readSerial() {
    arduino::String result;
    while (Serial.available()) {
      activity = true;
      if (sleep) {
        // Discard first input
        Serial.read();
        continue;
      }

      result += (char)Serial.read();
    }
    return result;
  }

  void receiveKeys() {
    inputBuffer += readSerial();
    // Backspace handling
    if ((inputBuffer.endsWith("\b") || inputBuffer.endsWith("\x7F")) && inputBuffer.length() >= 1) {
      if (inputBuffer.length() >= 2) {
        inputBuffer = inputBuffer.substring(0, inputBuffer.length() - 2);
      } else {
        inputBuffer = "";
      }
    }
    // Tab handling
    if (inputBuffer.endsWith("\t")) {
      if (inputBuffer.startsWith("/to ")) {
        auto partialName = inputBuffer.substring(4, inputBuffer.length() - 1);
        for (const auto &[_, contact] : client.contacts) {
          if (contact.advName.startsWith(partialName)) {
            inputBuffer = "/to " + contact.advName + " ";
            break;
          }
        }
      } else {
        inputBuffer = inputBuffer.substring(0, inputBuffer.length() - 1);
      }
    }
    // Command handling
    if (inputBuffer.endsWith("\r") || inputBuffer.endsWith("\n")) {
      auto command = inputBuffer.substring(0, inputBuffer.length() - 1);
      inputBuffer = "";

      if (command == "/help") {
        console.print("Available commands:");
        console.print("/clear - Clear the console");
        console.print("/self - Show self information");
        console.print("/list chan,contact - Request channel or contact list");
        return;
      }
      if (command == "/clear") {
        console.clear();
        return;
      }
      if (command.startsWith("/self")) {
        console.print(client.self.deviceName);
        console.print("Freq " + String(client.self.radioFreq) + " MHz");
        console.print("BW   " + String(client.self.radioBandwidth) + " kHz");
        console.print("SF   " + String(client.self.radioSpreadingFactor));
        console.print("CR   " + String(client.self.radioCodingRate));
      }
      if (command.startsWith("/list")) {
        if (command.endsWith("chan")) {
          for (int i = 0; i < client.channels.size(); i++) {
            auto &ch = client.channels.at(i);
            console.print("CH " + String(i) + ": " + ch.name);
          }
        } else if (command.endsWith("contact")) {
          for (const auto &[_, contact] : client.contacts) {
            console.print("CON: " + contact.advName);
          }
        }
      }
    }
  }

  void draw(GFX &gfx) {
    prompt = "> " + inputBuffer;

    gfx.drawText(16, gfx.height - 24, 1, const_cast<char *>(prompt.c_str()), prompt.length(), JUSTIFY_LEFT);

    // Flashing cursor
    if ((millis() / 100) % 2 == 0) {
      gfx.drawRect(16 + prompt.length() * 10, gfx.height - 24, 6, 12);
    }
  }

  bool activity = false;

private:
  MeshCore::Client &client;
  UI::Console &console;
  arduino::String prompt;
  arduino::String inputBuffer;
  bool &sleep;
};
} // namespace UI