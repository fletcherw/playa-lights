#ifndef FIRE_H
#define FIRE_H

#include "Pattern.h"

class Fire : public Pattern {
  public:
  Fire(CRGB *leds) :
    leds_(leds)
  {}

  void blit();
  int updateInterval();
  
private:
  CRGB getPixelHeatColor_(int pixel, byte temperature);
  
  CRGB *leds_;
  byte heat[24];
};

#endif
