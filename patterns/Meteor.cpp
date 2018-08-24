#include "Meteor.h"

void Meteor::blit() {
  // fade brightness all LEDs one step
  for (int j = 0; j < leds_.length(); j++) {
    if (random(10) > 3) {
      leds_[j].fadeToBlackBy(90);
    }
  }

  // draw meteor
  for (int j = 0; j < size_; j++) {
    if (0 <= index_ - j && index_ - j < leds_.length()) {
      leds_[index_ - j] += color_;
    }
  }
  index_ += delta_;
  if (index_ == -1 || index_ == leds_.length()) {
    delta_ *= -1;
    index_ += 2 * delta_;
  }
}

void Meteor::setColor(CRGB color) {
  color_ = color;
}
