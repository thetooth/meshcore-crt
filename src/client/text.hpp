#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace MeshCore::Text {

inline bool isContinuationByte(uint8_t value) {
  return (value & 0xC0) == 0x80;
}

inline arduino::String sanitizeUtf8ToAscii(const uint8_t *data, size_t length, bool stopAtNull = false,
                                           char replacement = '*') {
  arduino::String output;
  output.reserve(length);

  for (size_t index = 0; index < length;) {
    uint8_t first = data[index];

    if (stopAtNull && first == 0) {
      break;
    }

    if ((first & 0x80) == 0) {
      output += static_cast<char>(first);
      index++;
      continue;
    }

    uint32_t codepoint = 0;
    size_t expected = 0;

    if ((first & 0xE0) == 0xC0) {
      codepoint = first & 0x1F;
      expected = 2;
    } else if ((first & 0xF0) == 0xE0) {
      codepoint = first & 0x0F;
      expected = 3;
    } else if ((first & 0xF8) == 0xF0) {
      codepoint = first & 0x07;
      expected = 4;
    } else {
      output += replacement;
      index++;
      continue;
    }

    if ((index + expected) > length) {
      output += replacement;
      break;
    }

    bool valid = true;
    for (size_t i = 1; i < expected; i++) {
      uint8_t next = data[index + i];
      if ((stopAtNull && next == 0) || !isContinuationByte(next)) {
        valid = false;
        break;
      }
      codepoint = (codepoint << 6) | (next & 0x3F);
    }

    if (!valid || (expected == 2 && codepoint < 0x80) || (expected == 3 && codepoint < 0x800) ||
        (expected == 4 && codepoint < 0x10000) || (codepoint >= 0xD800 && codepoint <= 0xDFFF) ||
        codepoint > 0x10FFFF) {
      output += replacement;
      index += expected;
      continue;
    }

    output += (codepoint <= 0x7F) ? static_cast<char>(codepoint) : replacement;
    index += expected;
  }

  return output;
}

} // namespace MeshCore::Text