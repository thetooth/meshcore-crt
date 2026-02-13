#pragma once

extern "C" {
#include "connections.h"
#include "cvideo.h"
#include "renderer.h"
}

#include <functional>

class GFX {
public:
  GFX() {}

  void init(std::function<void(GFX &)> draw_callback) {
    // renderer_init([&]() { draw_callback(*this); });
  }

  void drawRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
  void drawText(unsigned int x, unsigned int y, unsigned int scale, char *text, unsigned int length,
                renderer_text_justify_t justification, bool invert = false);
  void drawImage(unsigned int x, unsigned int y, unsigned int width, unsigned int height, char *data,
                 bool invert = false);
  void drawCharacter(unsigned int x, unsigned int y, unsigned int scale, char character, bool invert = false);
  void clear();

  int width = CVIDEO_INTERLACED ? 640 : 320;
  int height = CVIDEO_INTERLACED ? 480 : 240;
  int screenXOffset = 0;
  int screenYOffset = 0;
  int screenLeft = (CVIDEO_PIX_PER_LINE - width) / 2 - screenXOffset;
  int screenRight = (CVIDEO_LINES - height) / 2 - screenYOffset;
};