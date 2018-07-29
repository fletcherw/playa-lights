#ifndef SPINNING_RAINBOW_H
#define SPINNING_RAINBOW_H

#include "Pattern.h"

class SpinningRainbow : public Pattern {
public:
  SpinningRainbow(CRGB *leds) :
    leds_(leds),
    patch_index_(0),
    hoop_index_(0),
    th_(0.0)
  {}

  void blit();
  int updateInterval();
  
private:
  CRGB *leds_;
  int num_leds_;
  int patch_index_;
  int hoop_index_;
  double th_;

  const double delta = 0.1;
  const double value_target = 80.0;
  const double value_radius = 15.0;
};

#endif
