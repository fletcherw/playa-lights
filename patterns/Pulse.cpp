#include "Pulse.h"

void Pulse::blit() {
  th_ += 0.1;
  if (th_ > 2 * PI) th_ -= 2 * PI;
  
  for (int i = 0; i < leds_.length(); i++) {
    leds_[i] = color_;
    double modifier = 180 * max((sin(th_) + .2)/1.2, 0.1);
    leds_[i] %= int(modifier);
  }
}

void Pulse::setColor(CRGB c) {
  color_ = c;
  blit();
}
