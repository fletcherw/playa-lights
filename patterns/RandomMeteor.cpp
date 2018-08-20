#include "RandomMeteor.h"

RandomMeteor::RandomMeteor(LEDSegment leds, int size) :
    leds_(leds),
    index_(0),
    delta_(1),
    size_(size)
{
  updateInterval_ = 60;
  color_ = CHSV(random(255), 255, 255);
}

void RandomMeteor::blit() {
  // fade brightness all LEDs one step
  for (int j = 0; j < leds_.length(); j++) {
    if (random(10) > 3) {
      leds_[j].fadeToBlackBy(90);
    }
  }

  // draw meteor
  for (int j = 0; j < size_; j++) {
    if (0 <= index_ - j && index_ - j < leds_.length()) {
      leds_[index_ - j] = color_;
    }
  }
  index_ += delta_;
  if (index_ == -1 || index_ == leds_.length()) {
    delta_ *= -1;
    index_ += 2 * delta_;
    color_ = CHSV(random(255), 255, 255); 
  }
}

CRGB RandomMeteor::getColor() {
  return color_;
}
