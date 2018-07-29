#include "SpinningRainbow.h"

void SpinningRainbow::blit() {
  hoop_index_++;
  patch_index_++;
  hoop_index_ %= 75;
  patch_index_ %= 20;
  
  th_ += delta;
  if (th_ > 2 * PI) th_ = 0.0;
  
  // set main led hoop
  for (int i = 0; i < 24; i++) {
    double hue = double((i + hoop_index_) % 75) / 75 * 255;
    double value = value_target + sin(th_) * value_radius;
    leds_[i] = CHSV(int(hue), 255, int(value));
  }
  // set back triple
  for (int i = 0; i < 5; i++) {
    double hue = double((i + patch_index_) % 20) / 20 * 255;
    double value = value_target + sin(th_) * value_radius;
    CRGB color = CHSV(int(hue), 255, int(value));
    leds_[28 - i] = color;
    leds_[29 + i] = color;
    leds_[38 - i] = color;
  }
}

int SpinningRainbow::updateInterval() { return 80; }
