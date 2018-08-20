#ifndef THEATER_CHASE_H
#define THEATER_CHASE_H

#include "Pattern.h"
#include "LEDSegment.h"

class TheaterChase : public Pattern {
public:
  TheaterChase(LEDSegment leds);

  void blit();
  void setColor(CRGB color);

private:
  LEDSegment leds_;
  uint8_t lightIndex_;  
  CRGB color_;
};

#endif
