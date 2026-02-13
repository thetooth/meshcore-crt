#pragma once

#include "../video/video.hpp"

namespace UI {
void screensaver(GFX &gfx) {
  // DVD screensaver
  static int x = gfx.width / 2;
  static int y = gfx.height / 2;
  static bool xDir;
  static bool yDir;
  static int size = 8;
  static int speed = 1;
  if (xDir) {
    x += speed;
  } else {
    x -= speed;
  }
  if (yDir) {
    y += speed;
  } else {
    y -= speed;
  }
  if (x <= 0 || x + size >= gfx.width) {
    xDir = !xDir;
  }
  if (y <= 0 || y + size >= gfx.height) {
    yDir = !yDir;
  }

  gfx.drawRect(x, y, size, size);
}
} // namespace UI