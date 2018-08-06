#include "Metronome.h"

void Metronome::blit() {
  hoopIndex_++;
  hoopIndex_ %= 2;
  
  for (int i = 0; i < 24; i++) {
    if (i % 2 == hoopIndex_) leds_[i] = color_;
    else leds_[i] = CRGB::Black;
  }

  for (int i = 0; i < 5; i++) leds_[24 + (5 * armIndex_) + i] = CRGB::Black;
  armIndex_ += armDelta_;
  if (armIndex_ == 3) {
    armIndex_ = 1;
    armDelta_ = -1;
  }
  
  if (armIndex_ == -1) {
    armIndex_ = 1;
    armDelta_ = 1;
  }
  for (int i = 0; i < 5; i++) leds_[24 + (5 * armIndex_) + i] = color_;
}

int Metronome::updateInterval() { return 200; }

void Metronome::setColor(CRGB c) {
  color_ = c;
  blit();
}

