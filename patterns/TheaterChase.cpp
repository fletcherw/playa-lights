#include "TheaterChase.h"

TheaterChase::TheaterChase(LEDSegment leds) :
  leds_(leds),
  lightIndex_(0),
  color_(CRGB::White)
{
  updateInterval_ = 100;
}

void TheaterChase::blit() {
  lightIndex_ += 1;
  lightIndex_ %= 3;

  for (int i = 0; i < leds_.length(); i++) {
    if (i % 3 == lightIndex_) {
      leds_[i] = color_; 
    } else {
      leds_[i] = CRGB::Black;
    }
  }
}

void TheaterChase::setColor(CRGB color) {
  color_ = color;
  blit();
}
