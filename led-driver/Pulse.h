#ifndef PULSE_H
#define PULSE_H

#include "Pattern.h"

class Pulse : public Pattern {
public:
  Pulse(CRGB *leds) :
    th_(0.0),
    leds_(leds),
    color_(CRGB::White)
  {}

  void blit();
  int updateInterval();
  void setColor(CRGB c);
  
private:
  double th_;
  CRGB *leds_;
  CRGB color_;
};

#endif
