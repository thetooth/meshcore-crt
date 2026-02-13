#include <Arduino.h>

#define _TASK_MICRO_RES
#define _TASK_TIMECRITICAL
#define _TASK_SLEEP_ON_IDLE_RUN

#include "client/api.hpp"
#include "client/serial.hpp"
#include "pico/stdlib.h"
#include "ui/console.hpp"
#include "ui/screensaver.hpp"
#include "video/video.hpp"

#include <TaskScheduler.h>
#include <stdio.h>
#include <stdlib.h>

void draw(void);
void terminalClient();

#define PADDING             10
#define SLEEP_TIMEOUT_MS    30000
#define ACTIVITY_TIMEOUT_MS 1000

char MSG_COLDBOOT[] = "AWAITING TELEMETRY";

GFX gfx;
UI::Console console(gfx);
MeshCore::Client client(console);
MeshCore::SerialInterface serialInterface(client);

bool coldBoot = false;
bool sleep = false;
unsigned long sleepTimeout = 0;
unsigned long activityTimeout = 0;

Scheduler ts;
// Task t1(10 * 1000, TASK_FOREVER, terminalClient, &ts, true);
Task t2(1000, TASK_FOREVER, []() { serialInterface.receiveRadio(); }, &ts, true);
Task t3(
    1000 * 1000, TASK_FOREVER,
    []() {
      if (client.msgWaiting) {
        client.requestNextMessage();
      }
    },
    &ts, true);
Task channelInfoTask(
    1000, TASK_FOREVER,
    []() {
      if (client.requestAllChannels()) {
        channelInfoTask.disable();
      }
    },
    &ts, false);

void setup() {
  Serial.begin(115200);
  sleep_ms(1000);
  Serial.println("Booting Meshcore Cyberdeck...");

  // gfx.init(draw);

  renderer_init(draw);
  Serial.println("Renderer initialised");

  Serial1.setPinout(16, 17);
  Serial1.setFIFOSize(512);
  Serial1.begin(115200);

  sleepTimeout = millis() + SLEEP_TIMEOUT_MS;

  client.appStart();
  channelInfoTask.delay(2000 * 1000);
  channelInfoTask.enable();
  client.requestContacts();

  while (1) {
    // serialInterface.receiveRadio();
    if (client.activity) {
      sleepTimeout = millis() + SLEEP_TIMEOUT_MS;
      activityTimeout = millis() + ACTIVITY_TIMEOUT_MS;
      if (sleep) {
        sleep = false;
      }
      client.activity = false;
    }

    ts.execute();
    renderer_run();
  }
}

void loop() {}

arduino::String inputBuffer;
arduino::String readSerial() {
  arduino::String result;
  while (Serial.available()) {
    sleepTimeout = millis() + SLEEP_TIMEOUT_MS;
    if (sleep) {
      sleep = false;

      // Discard first input
      Serial.read();
      continue;
    }

    result += (char)Serial.read();
  }
  return result;
}

void terminalClient() {
  inputBuffer += readSerial();
  // Backspace handling
  if ((inputBuffer.endsWith("\b") || inputBuffer.endsWith("\x7F")) && inputBuffer.length() > 1) {
    inputBuffer = inputBuffer.substring(0, inputBuffer.length() - 2);
  }
  // Command handling
  if (inputBuffer.endsWith("\r") || inputBuffer.endsWith("\n")) {
    auto command = inputBuffer.substring(0, inputBuffer.length() - 1);
    inputBuffer = "";

    console.print("> ");

    if (command == "/clr") {
      console.lines.fill("");
      return;
    }
    if (command == "/sleep") {
      gfx.clear();
      sleep = true;
      return;
    }

    Serial1.print(command + "\r\n");
  }

  if (Serial1.available()) {
    coldBoot = false;
    sleepTimeout = millis() + SLEEP_TIMEOUT_MS;
    if (sleep) {
      sleep = false;
    }

    auto c = (char)Serial1.read();
    Serial.print(c);
    if (c == '\n') {
      console.ln();
      return;
    }
    if (c == '\r') {
      return;
    }
    console.append(c);
  }
}

arduino::String prompt;
uint32_t frameCount = 0;
void draw(void) {
  auto t0 = millis();
  frameCount++;
  if (millis() > sleepTimeout) {
    // sleep = true;
  }
  if (sleep) {
    UI::screensaver(gfx);
    return;
  }

  if (!coldBoot) {
    // Draw 20x20 grid
    unsigned int sz = frameCount % 3 == 0 && millis() < activityTimeout ? 8 : 2;
    for (int x = 32; x < gfx.width - PADDING; x += 32) {
      for (int y = 32; y < gfx.height - PADDING; y += 32) {
        gfx.drawRect(x, y, sz, sz);
      }
    }

    // // Render area box
    // gfx.drawRect(0, 0, gfx.width, 1);
    // gfx.drawRect(0, gfx.height, gfx.width, 1);

    // gfx.drawRect(0, 0, 2, gfx.height);
    // gfx.drawRect(gfx.width, 0, 2, gfx.height);
  }

  if (coldBoot && frameCount % 4 == 0) {
    gfx.drawText(gfx.width / 2, gfx.height / 2, 3, MSG_COLDBOOT, sizeof(MSG_COLDBOOT) - 1, JUSTIFY_CENTRE);
  }

  console.draw();

  // prompt = "> " + inputBuffer;

  // gfx.drawText(16, gfx.height - 48, 2, const_cast<char *>(prompt.c_str()), prompt.length(), JUSTIFY_LEFT);

  // // Flashing cursor
  // if ((millis() / 100) % 2 == 0) {
  //   gfx.drawRect(16 + prompt.length() * 20, gfx.height - 48, 12, 24);
  // }
  auto t1 = millis() - t0;
  auto fps = arduino::String(t1) + "ms";
  gfx.drawText(gfx.width - 16, gfx.height - 48, 1, const_cast<char *>(fps.c_str()), fps.length(),
               JUSTIFY_RIGHT);
}