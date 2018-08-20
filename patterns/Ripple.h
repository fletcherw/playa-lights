#ifndef RIPPLE_H
#define RIPPLE_H

#include "Pattern.h"
#include "LEDSegment.h"

class Ripple : public Pattern {
public:
  Ripple(LEDSegment leds);

  void blit();
  void setColor(CRGB color);

private:
  LEDSegment leds_;
  CRGB color_;
  int index_;
  int radius_;

  long lastBlit_;
  long totalDarkTime_;
};

#endif
