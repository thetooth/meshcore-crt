#include "video.hpp"

void GFX::drawRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
  renderer_draw_rect(x + screenLeft, y + screenRight, width, height);
}

void GFX::drawText(unsigned int x, unsigned int y, unsigned int scale, char *text, unsigned int length,
                   renderer_text_justify_t justification, bool invert) {
  renderer_draw_string(x + screenLeft, y + screenRight, scale, text, length, justification, invert);
}

void GFX::drawImage(unsigned int x, unsigned int y, unsigned int width, unsigned int height, char *data,
                    bool invert) {
  renderer_draw_image(x + screenLeft, y + screenRight, width, height, data, invert);
}

void GFX::drawCharacter(unsigned int x, unsigned int y, unsigned int scale, char character, bool invert) {
  renderer_draw_character(x + screenLeft, y + screenRight, scale, character, invert);
}

void GFX::clear() {
  renderer_clear_default();
}