#pragma once

#include "../video/video.hpp"

#include <Arduino.h>
#include <array>

namespace UI {
class Console {
public:
  Console() { clear(); }

  void draw(GFX &gfx) {
    const int renderedLines = (lineCount < maxLines) ? lineCount : maxLines;
    int start = lineCount - renderedLines - scrollOffset;
    if (start < 0) {
      start = 0;
    }

    for (int i = 0; i < renderedLines; i++) {
      const int index = start + i;
      gfx.drawText(0, 16 + (i * 16), 1, const_cast<char *>(buffer[index].c_str()), buffer[index].length(),
                   JUSTIFY_LEFT);
    }
  }

  void print(const arduino::String &text, bool newLine = true, bool wrap = true) {
    if (newLine) {
      ln();
    }
    if (wrap) {
      wrapText(text);
    } else {
      currentLine() = text.substring(0, maxCharsPerLine);
    }
  }

  void append(const char c) {
    if (currentLine().length() >= maxCharsPerLine) {
      ln();
    }
    currentLine() += c;
  }

  void ln() {
    if (lineCount < maxBufferLines) {
      lineCount++;
    } else {
      for (int i = 0; i < maxBufferLines - 1; i++) {
        buffer[i] = buffer[i + 1];
      }
      if (scrollOffset > 0) {
        scrollOffset--;
      }
    }

    currentLine() = "";
    clampScrollOffset();
  }

  void clear() {
    buffer.fill("");
    lineCount = 1;
    scrollOffset = 0;
  }

  void scrollUp() {
    const int maxOffset = maxScrollOffset();
    if (scrollOffset < maxOffset) {
      scrollOffset++;
    }
  }

  void scrollDown() {
    if (scrollOffset > 0) {
      scrollOffset--;
    }
  }

  void scrollReset() { scrollOffset = 0; }

private:
  arduino::String &currentLine() { return buffer[lineCount - 1]; }

  int maxScrollOffset() const {
    const int maxOffset = lineCount - maxLines;
    return (maxOffset > 0) ? maxOffset : 0;
  }

  void clampScrollOffset() {
    if (scrollOffset < 0) {
      scrollOffset = 0;
    }

    const int maxOffset = maxScrollOffset();
    if (scrollOffset > maxOffset) {
      scrollOffset = maxOffset;
    }
  }

  void wrapWord(const arduino::String &word) {
    if (word.length() == 0) {
      return;
    }

    if (currentLine().length() == 0) {
      if (word.length() <= maxCharsPerLine) {
        currentLine() = word;
        return;
      }

      arduino::String remaining = word;
      while (remaining.length() > maxCharsPerLine) {
        currentLine() = remaining.substring(0, maxCharsPerLine);
        remaining = remaining.substring(maxCharsPerLine);
        ln();
      }
      currentLine() = remaining;
      return;
    }

    if (currentLine().length() + 1 + word.length() <= maxCharsPerLine) {
      currentLine() += " ";
      currentLine() += word;
      return;
    }

    ln();
    wrapWord(word);
  }

  void wrapText(const arduino::String &text) {
    arduino::String word;

    for (int i = 0; i < text.length(); i++) {
      const char c = text[i];

      if (c == ' ' || c == '\t') {
        wrapWord(word);
        word = "";
        continue;
      }

      if (c == '\r' || c == '\n') {
        wrapWord(word);
        word = "";

        if (c == '\r' && (i + 1) < text.length() && text[i + 1] == '\n') {
          i++;
        }
        ln();
        continue;
      }

      word += c;
    }

    wrapWord(word);
  }

  static const int maxLines = 12;
  static const int maxBufferLines = 256;
  static const int maxCharsPerLine = 30;
  std::array<arduino::String, maxBufferLines> buffer;
  int lineCount = 1;
  int scrollOffset = 0;
};
} // namespace UI