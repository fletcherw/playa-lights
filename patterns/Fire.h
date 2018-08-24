#ifndef FIRE_H
#define FIRE_H

#include "LEDSegment.h"
#include "Pattern.h"

class Fire : public Pattern {
public:
  Fire(LEDSegment leds, int cooling=15);
  ~Fire();
  void blit();
  
private:
  CRGB getPixelHeatColor_(int pixel, byte temperature);
  
  LEDSegment leds_;
  byte* heat_;
  int cooling_;
};

#endif
