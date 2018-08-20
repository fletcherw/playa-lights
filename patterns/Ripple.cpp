#include "Ripple.h"

Ripple::Ripple(LEDSegment leds) :
  leds_(leds),
  color_(CRGB::White),
  index_(-1)
  {
    lastBlit_ = millis();
    blit();
    updateInterval_ = 100;
  }

void Ripple::blit() {
  if (index_ == -1) {
    totalDarkTime_ += millis() - lastBlit_; 
    lastBlit_ = millis();
    if (totalDarkTime_ > 2000) {
      index_ = random(leds_.length());
      leds_[index_] = color_;
      radius_ = 0;
    }
  } else {
    radius_++;
    if (radius_ == 6) {
      for (int i = index_ - radius_; i <= index_ + radius_; i++) {
        if (!(0 <= i && i < leds_.length())) continue;
        leds_[i] = CRGB::Black;
      }
      index_ = -1;
      totalDarkTime_ = 0;
      lastBlit_ = millis();
    } else {
      if (index_ - radius_ >= 0) leds_[index_ - radius_] = leds_[index_];
      if (index_ + radius_ < leds_.length()) leds_[index_ + radius_] = leds_[index_];
      for (int i = index_ - radius_; i <= index_ + radius_; i++) {
        if (!(0 <= i && i < leds_.length())) continue;
        leds_[i] %= 140;
      }
    }
  }
}

void Ripple::setColor(CRGB color) {
  color_ = color;
}
