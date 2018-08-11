#include "SpinningRainbow.h"

void SpinningRainbow::blit() {
  index_--;
  index_ %= radius_;

  th_ += delta;
  if (th_ > 2 * PI) th_ = 0.0;
  for (int i = 0; i < leds_.length(); i++) {
    double hue = double((i + index_) % radius_) / radius_ * 255;
    double value = valueTarget + sin(th_) * valueRadius;
    leds_[i] = CHSV(int(hue), 255, int(value));
  }
}
