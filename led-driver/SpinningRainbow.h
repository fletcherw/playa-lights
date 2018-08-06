#ifndef SPINNING_RAINBOW_H
#define SPINNING_RAINBOW_H

#include "Pattern.h"

class SpinningRainbow : public Pattern {
public:
  SpinningRainbow(CRGB *leds) :
    leds_(leds),
    patchIndex_(0),
    hoopIndex_(0),
    th_(0.0)
  {}

  void blit();
  int updateInterval();
  
private:
  CRGB *leds_;
  int numLeds_;
  int patchIndex_;
  int hoopIndex_;
  double th_;

  const double delta = 0.1;
  const double valueTarget = 80.0;
  const double valueRadius = 15.0;
};

#endif
