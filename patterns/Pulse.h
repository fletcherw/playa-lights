#ifndef PULSE_H
#define PULSE_H

#include "LEDSegment.h"
#include "Pattern.h"

class Pulse : public Pattern {
public:
  Pulse(LEDSegment leds) :
    th_(0.0),
    leds_(leds),
    color_(CRGB::White)
  {}

  void blit();
  void setColor(CRGB c);
  
private:
  double th_;
  LEDSegment leds_;
  CRGB color_;
};

#endif
