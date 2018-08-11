#ifndef PING_PONG_H
#define PING_PONG_H

#include "LEDSegment.h"
#include "Pattern.h"

class PingPong : public Pattern {
public:
  PingPong(LEDSegment leds) :
    leds_(leds),
    color_(CRGB(40, 0, 120)),
    index_(0),
    delta_(1)
  {}

  void blit();
  int updateInterval();
  void setColor(CRGB c);
  
private:
  LEDSegment leds_;
  CRGB color_;
  int index_;
  int delta_;
};

#endif
