#include "SpinningRainbow.h"

void SpinningRainbow::blit() {
  index_++;
  th_ += delta;
  if (th_ > 2 * PI) th_ = 0.0;
  for (int i = 0; i < num_leds_; i++) {
    double hue = double(i) / num_leds_ * 255;
    double value = value_target + sin(th_) * value_radius;
    leds_[(i + index_) % num_leds_] = CHSV(int(hue), 255, int(value));
  }
}
