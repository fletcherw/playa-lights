#ifndef SOLID_H
#define SOLID_H

#include "LEDSegment.h"
#include "Pattern.h"

class Solid : public Pattern {
public:
  Solid(LEDSegment leds) :
    leds_(leds),
    color_(CRGB::White)
  {
    updateInterval_ = -1; 
  }

  void blit();
  void setColor(CRGB c);
  
private:
  LEDSegment leds_;
  CRGB color_;
};

#endif
