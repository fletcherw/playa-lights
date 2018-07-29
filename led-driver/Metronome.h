#ifndef METRONOME_H
#define METRONOME_H

#include "Pattern.h"

class Metronome : public Pattern {
public:
  Metronome(CRGB *leds) :
    leds_(leds),
    hoop_index_(0),
    hoop_delta_(1),
    arm_index_(1),
    arm_delta_(1)
  {}

  void blit();
  int updateInterval();
  
private:
  CRGB *leds_;
  int hoop_index_;
  int hoop_delta_;
  int arm_index_;
  int arm_delta_;
};

#endif
