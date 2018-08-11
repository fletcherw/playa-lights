#include "SpinningRainbow.h"

void SpinningRainbow::blit() {
  hoopIndex_++;
  patchIndex_++;
  hoopIndex_ %= 75;
  patchIndex_ %= 20;
  
  th_ += delta;
  if (th_ > 2 * PI) th_ = 0.0;
  
  // set main led hoop
  for (int i = 0; i < 24; i++) {
    double hue = double((i + hoopIndex_) % 75) / 75 * 255;
    double value = valueTarget + sin(th_) * valueRadius;
    leds_[i] = CHSV(int(hue), 255, int(value));
  }
  // set back triple
  for (int i = 0; i < 5; i++) {
    double hue = double((i + patchIndex_) % 20) / 20 * 255;
    double value = valueTarget + sin(th_) * valueRadius;
    CRGB color = CHSV(int(hue), 255, int(value));
    leds_[28 - i] = color;
    leds_[29 + i] = color;
    leds_[38 - i] = color;
  }
}

int SpinningRainbow::updateInterval() { return 80; }
