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
    inputBuffer += filterSerialInput(readSerial());
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
      console.scrollReset();

      if (command.startsWith("/")) {
        if (command == "/help") {
          console.print("Available commands:");
          console.print("/clear - Clear the console");
          console.print("/self - Show self information");
          console.print("/list channel,user,repeater - Request channel, user, or repeater list");
          return;
        } else if (command == "/clear") {
          console.clear();
          return;
        } else if (command.startsWith("/self")) {
          console.print(client.self.deviceName);
          console.print("Freq " + String(client.self.radioFreq) + " MHz");
          console.print("BW   " + String(client.self.radioBandwidth) + " kHz");
          console.print("SF   " + String(client.self.radioSpreadingFactor));
          console.print("CR   " + String(client.self.radioCodingRate));
        } else if (command.startsWith("/list")) {
          if (command.endsWith("channel")) {
            for (int i = 0; i < client.channels.size(); i++) {
              auto &ch = client.channels.at(i);
              console.print("CH " + String(i) + ": " + ch.name);
            }
          } else if (command.endsWith("user")) {
            for (const auto &[_, contact] : client.contacts) {
              if (contact.type != 1) {
                continue;
              }
              console.print("CON: " + contact.advName);
            }
          } else if (command.endsWith("repeater")) {
            for (const auto &[_, contact] : client.contacts) {
              if (contact.type != 2) {
                continue;
              }
              console.print("REP: " + contact.advName);
            }
          }
        } else if (command.startsWith("/to ")) {
          arduino::String pubKeyPrefix = "";
          auto recipient = command.substring(4);
          recipient.trim();
          // Find in contacts
          for (const auto &[_, contact] : client.contacts) {
            if (contact.advName == recipient) {
              pubKeyPrefix = arduino::String(contact.pubKey, 6);
              break;
            }
          }

          if (pubKeyPrefix.length() == 0) {
            console.print("Not found: " + recipient);
            return;
          }

          client.toRecipient = pubKeyPrefix;
          console.print("Set to " + recipient);
          return;
        } else {
          console.print("Unknown command");
          return;
        }
      } else {
        // Message sending
        if (command.length() > 0) {
          if (client.toRecipient.length() <= 0) {
            console.print("No recipient specified.");
          } else {
            client.sendTextMessage(command);

            console.print("DIRECT " + client.self.deviceName + ":");
            console.print(command);
          }
        }
      }
    }
  }

  void draw(GFX &gfx) {
    prompt = "> " + inputBuffer;

    gfx.drawText(0, gfx.height - 24, 1, const_cast<char *>(prompt.c_str()), prompt.length(), JUSTIFY_LEFT);

    // Flashing cursor
    if ((millis() / 100) % 2 == 0) {
      gfx.drawRect(prompt.length() * 10, gfx.height - 24, 6, 12);
    }
  }

  bool activity = false;

private:
  arduino::String filterSerialInput(const arduino::String &input) {
    arduino::String filtered;

    for (int i = 0; i < input.length(); i++) {
      if (input[i] == '\x1b') {
        if ((i + 2) < input.length() && input[i + 1] == '[' && input[i + 2] == 'A') {
          console.scrollUp();
          i += 2;
          continue;
        }

        if ((i + 2) < input.length() && input[i + 1] == '[' && input[i + 2] == 'B') {
          console.scrollDown();
          i += 2;
          continue;
        }

        if ((i + 3) < input.length() && input[i + 1] == '[' && input[i + 2] == '5' && input[i + 3] == '~') {
          console.scrollUp();
          i += 3;
          continue;
        }

        if ((i + 3) < input.length() && input[i + 1] == '[' && input[i + 2] == '6' && input[i + 3] == '~') {
          console.scrollDown();
          i += 3;
          continue;
        }
      }

      filtered += input[i];
    }

    return filtered;
  }

  MeshCore::Client &client;
  UI::Console &console;
  arduino::String prompt;
  arduino::String inputBuffer;
  bool &sleep;
};
} // namespace UI