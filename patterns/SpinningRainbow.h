#ifndef SPINNING_RAINBOW_H
#define SPINNING_RAINBOW_H

#include "LEDSegment.h"
#include "Pattern.h"

class SpinningRainbow : public Pattern {
public:
  SpinningRainbow(LEDSegment leds, int circleRadius) :
    leds_(leds),
    index_(0),
    radius_(circleRadius),
    th_(0.0)
  {
    updateInterval_ = 80; 
  }

  void blit();
  
private:
  LEDSegment leds_;
  int index_;
  int radius_;
  double th_;

  const double delta = 0.1;
  const double valueTarget = 80.0;
  const double valueRadius = 15.0;
};

#endif
