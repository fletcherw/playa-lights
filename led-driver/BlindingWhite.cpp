#include "BlindingWhite.h"

void BlindingWhite::blit() {
  for (int i = 0; i < 39; i++) {
    leds_[i] = CRGB::White;
    leds_[i] %= 240;
  }
}

int BlindingWhite::updateInterval() { return -1; }
