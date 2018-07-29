#include "Metronome.h"

void Metronome::blit() {
  hoop_index_++;
  hoop_index_ %= 2;
  
  for (int i = 0; i < 24; i++) {
    if (i % 2 == hoop_index_) leds_[i] = CRGB::Blue;
    else leds_[i] = CRGB::Black;
  }

  for (int i = 0; i < 5; i++) leds_[24 + (5 * arm_index_) + i] = CRGB::Black;
  arm_index_ += arm_delta_;
  if (arm_index_ == 3) {
    arm_index_ = 1;
    arm_delta_ = -1;
  }
  
  if (arm_index_ == -1) {
    arm_index_ = 1;
    arm_delta_ = 1;
  }
  for (int i = 0; i < 5; i++) leds_[24 + (5 * arm_index_) + i] = CRGB::Blue;
}

int Metronome::updateInterval() { return 200; }
