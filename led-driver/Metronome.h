#ifndef METRONOME_H
#define METRONOME_H

#include "Pattern.h"

class Metronome : public Pattern {
public:
  Metronome(CRGB *leds) :
    leds_(leds),
    color_(CRGB::Blue),
    hoopIndex_(0),
    hoopDelta_(1),
    armIndex_(1),
    armDelta_(1)
  {}

  void blit();
  int updateInterval();
  void setColor(CRGB c);
  
private:
  CRGB *leds_;
  CRGB color_;
  int hoopIndex_;
  int hoopDelta_;
  int armIndex_;
  int armDelta_;
};

#endif
