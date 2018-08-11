#include "PingPong.h"

void PingPong::blit() {
  leds_[index_] = CRGB::Black;
  index_ += delta_;
  if (index_ >= 39) {
    delta_ = -1;
    index_ = 39 - 2;
  } else if (index_ < 0) {
    delta_ = 1;
    index_ = 1;
  }
  
  leds_[index_] = color_;
}

int PingPong::updateInterval() { return 50; }

void PingPong::setColor(CRGB c) {
  color_ = c;
  blit();
}
