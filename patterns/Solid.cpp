#include "Solid.h"

void Solid::blit() {
  for (int i = 0; i < leds_.length(); i++) {
    leds_[i] = color_;
    leds_[i] %= 240;
  }
}

void Solid::setColor(CRGB c) {
  color_ = c;
  blit();
}

