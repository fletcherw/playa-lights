#ifndef METEOR_H
#define METEOR_H

#include "LEDSegment.h"
#include "Pattern.h"

class Meteor : public Pattern {
public:
  Meteor(LEDSegment leds, int size = 3) :
    leds_(leds),
    index_(0),
    delta_(1),
    size_(size),
    color_(CRGB::White)
  {
    updateInterval_ = 60;
  }

  void blit();
  void setColor(CRGB c);
  
private:
  LEDSegment leds_;
  int index_;
  int delta_;
  int size_;
  CRGB color_;
};

#endif
