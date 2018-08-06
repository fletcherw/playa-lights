#include "Solid.h"

void Solid::blit() {
  for (int i = 0; i < 39; i++) {
    leds_[i] = color_;
    leds_[i] %= 240;
  }
}

int Solid::updateInterval() { return -1; }

void Solid::setColor(CRGB c) {
  color_ = c;
  blit();
}

