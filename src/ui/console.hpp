#pragma once

#include "../video/video.hpp"

#include <Arduino.h>
#include <array>

namespace UI {
class Console {
public:
  Console(GFX &gfx) : gfx(gfx) {}

  void draw() {
    for (int i = 0; i < lines.size(); i++) {
      gfx.drawText(16, 16 + (i * 16), 1, const_cast<char *>(lines[i].c_str()), lines[i].length(),
                   JUSTIFY_LEFT);
    }
  }

  void print(const arduino::String &text, bool newLine = true) {
    if (newLine) {
      ln();
    }
    lines[lines.size() - 1] = text.substring(0, maxCharsPerLine);
  }

  void append(const char c) {
    if (lines[lines.size() - 1].length() >= maxCharsPerLine) {
      ln();
    }
    lines[lines.size() - 1] += c;
  }

  void ln() {
    for (int i = 0; i < lines.size() - 1; i++) {
      lines[i] = lines[i + 1];
    }
    lines[lines.size() - 1] = "";
  }

  GFX &gfx;
  static const int maxLines = 13;
  static const int maxCharsPerLine = 30;
  std::array<arduino::String, maxLines> lines;
};
} // namespace UI