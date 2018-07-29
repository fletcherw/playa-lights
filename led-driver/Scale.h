#ifndef SCALE_H
#define SCALE_H

#include "Pattern.h"

class Scale : public Pattern {
public:
  Scale(CRGB *leds, int num_leds) :
    leds_(leds),
    num_leds_(num_leds),
    color_(CRGB(0, 120, 120))
  {}

  void blit();
  
private:
  CRGB *leds_;
  int num_leds_;
  CRGB color_;
};

#endif
