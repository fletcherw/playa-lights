#ifndef SPINNING_RAINBOW_H
#define SPINNING_RAINBOW_H

#include "Pattern.h"

class SpinningRainbow : public Pattern {
public:
  SpinningRainbow(CRGB *leds, int num_leds) :
    leds_(leds),
    num_leds_(num_leds),
    index_(0),
    th_(0.0)
  {}

  void blit();
  
private:
  CRGB *leds_;
  int num_leds_;
  int index_;
  double th_;

  const double delta = 0.1;
  const double value_target = 80.0;
  const double value_radius = 15.0;
};

#endif
