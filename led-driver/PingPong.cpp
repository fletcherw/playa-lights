#include "PingPong.h"

void PingPong::blit() {
  leds_[index_] = CRGB::Black;
  index_ += delta_;
  if (index_ >= num_leds_) {
    delta_ = -1;
    index_ = num_leds_ - 2;
  } else if (index_ < 0) {
    delta_ = 1;
    index_ = 1;
  }
  
  leds_[index_].r = 40;
  leds_[index_].b = 120;
}
