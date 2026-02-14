#include <Arduino.h>

#define _TASK_MICRO_RES
#define _TASK_TIMECRITICAL
#define _TASK_SLEEP_ON_IDLE_RUN

#include "client/api.hpp"
#include "client/serial.hpp"
#include "pico/stdlib.h"
#include "ui/console.hpp"
#include "ui/prompt.hpp"
#include "ui/screensaver.hpp"
#include "video/video.hpp"

#include <TaskScheduler.h>
#include <stdio.h>
#include <stdlib.h>

void draw(void);

#define PADDING             10
#define KEY_DEBOUNCE_US     10 * 1000
#define MSG_POLL_US         1000 * 1000
#define LOGO_TIME_MS        1000
#define SLEEP_TIMEOUT_MS    60000
#define ACTIVITY_TIMEOUT_MS 500

bool sleep = false;
unsigned long logoTimeout = 0;
unsigned long sleepTimeout = 0;
unsigned long activityTimeout = 0;

GFX gfx;
UI::Console console;

MeshCore::Client client(console);
MeshCore::SerialInterface serialInterface(client);

UI::Prompt prompt(client, console, sleep);

Scheduler ts;
Task t1(KEY_DEBOUNCE_US, TASK_FOREVER, []() { prompt.receiveKeys(); }, &ts, true);
Task t2(
    MSG_POLL_US, TASK_FOREVER, []() { client.msgWaiting ? client.requestNextMessage() : (void)0; }, &ts,
    true);

void setup() {
  Serial.begin(115200);
  sleep_ms(1000);
  Serial.println("Booting Meshcore Cyberdeck...");

  // gfx.init(draw);

  renderer_init(draw);
  Serial.println("Renderer initialised");

  Serial1.setPinout(16, 17);
  Serial1.setFIFOSize(256);
  Serial1.begin(115200);

  sleepTimeout = millis() + SLEEP_TIMEOUT_MS;
  logoTimeout = millis() + LOGO_TIME_MS;
  t2.delay(3 * MSG_POLL_US);

  client.appStart();
  client.requestChannels();
  client.requestContacts();

  while (1) {
    // Handle incoming radio data
    serialInterface.receiveRadio();

    // Run scheduled tasks (e.g., prompt input handling, message polling)
    ts.execute();

    // Power management / notifications
    if (client.activity) {
      activityTimeout = millis() + ACTIVITY_TIMEOUT_MS;
      console.scrollReset();
    }

    if (client.activity || prompt.activity) {
      sleepTimeout = millis() + SLEEP_TIMEOUT_MS;
      if (sleep) {
        sleep = false;
      }
      client.activity = false;
      prompt.activity = false;
    }

    renderer_run();
  }
}

void loop() {}

uint32_t frameCount = 0;
void draw(void) {
  auto t0 = millis();
  frameCount++;

  if (millis() > sleepTimeout) {
    sleep = true;
  }
  if (sleep) {
    UI::screensaver(gfx);
    return;
  }

  arduino::String header = "";
  if (client.sendQueue.size() > 0 && millis() < client.sendTimeout) {
    header += "SENDING";
  }
  if (frameCount % 60 < 30) {
    gfx.drawText(gfx.width, 0, 1, const_cast<char *>(header.c_str()), header.length(), JUSTIFY_RIGHT, true);
  }

  if (millis() > logoTimeout) {
    // Draw 20x20 grid
    unsigned int sz = frameCount % 3 == 0 && millis() < activityTimeout ? 3 : 0;
    for (int x = 16; x < gfx.width - PADDING; x += 16) {
      for (int y = 16; y < gfx.height - PADDING; y += 16) {
        gfx.drawRect(x, y, sz, sz);
      }
    }

    if (sz != 0) {
      // Render area box
      gfx.drawRect(0, 0, gfx.width, 1);
      gfx.drawRect(0, gfx.height, gfx.width, 1);

      gfx.drawRect(0, 0, 2, gfx.height);
      gfx.drawRect(gfx.width, 0, 2, gfx.height);
    }

  } else if (frameCount % 4 == 0) {
    gfx.drawImage((gfx.width - meshcore_width) / 2, (gfx.height - meshcore_height) / 2, meshcore_width,
                  meshcore_height, (char *)meshcore_bits, true);
    activityTimeout = millis() + ACTIVITY_TIMEOUT_MS;
  }

  console.draw(gfx);

  prompt.draw(gfx);

#ifdef DEBUG
  auto t1 = millis() - t0;
  auto fps = arduino::String(t1) + "ms";
  gfx.drawText(gfx.width - 16, gfx.height - 48, 1, const_cast<char *>(fps.c_str()), fps.length(),
               JUSTIFY_RIGHT);
#endif
}