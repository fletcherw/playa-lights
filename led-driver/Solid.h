#ifndef SOLID_H
#define SOLID_H

#include "Pattern.h"

class Solid : public Pattern {
public:
  Solid(CRGB *leds) :
    leds_(leds),
    color_(CRGB::White)
  {}

  void blit();
  int updateInterval();
  void setColor(CRGB c);
  
private:
  CRGB *leds_;
  CRGB color_;
};

#endif
