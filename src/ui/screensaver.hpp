#pragma once

#include "../video/cat.xbm"
#include "../video/meshcore.xbm"
#include "../video/screensaver.xbm"
#include "../video/video.hpp"

namespace UI {
void screensaver(GFX &gfx) {
  // DVD screensaver
  static int x = gfx.width / 2 - screensaver_width / 2;
  static int y = gfx.height / 2 - screensaver_height / 2;
  static bool xDir;
  static bool yDir;
  static int sizeX = screensaver_width;
  static int sizeY = screensaver_height;
  static int speed = 1;

  // gfx.drawImage(0, 0, cat_width, cat_height, (char *)cat_bits);
  // return;

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
  if (x <= 0 || x + sizeX >= gfx.width) {
    xDir = !xDir;
  }
  if (y <= 0 || y + sizeY >= gfx.height) {
    yDir = !yDir;
  }

  gfx.drawImage(x, y, screensaver_width, screensaver_height, (char *)screensaver_bits, true);

  // gfx.drawRect(x, y, size, size);
}
} // namespace UI