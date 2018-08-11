#ifndef SPARKLE_H
#define SPARKLE_H

#include "LEDSegment.h"
#include "Pattern.h"

class Sparkle : public Pattern {
public:
  Sparkle(LEDSegment leds);
  ~Sparkle();

  void blit();
  
private:
  struct pixelState {
    int age;
    int lifetime;
  };
  
  CRGB calculateColor_(const pixelState& state);

  LEDSegment leds_;
  pixelState* states_;
  long lastBlit_;
  CRGB color_;
};

#endif
