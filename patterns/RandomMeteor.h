#ifndef RANDOM_METEOR_H
#define RANDOM_METEOR_H

#include "LEDSegment.h"
#include "Pattern.h"

class RandomMeteor : public Pattern {
public:
  RandomMeteor(LEDSegment leds, int size = 3);
  void blit();
  CRGB getColor();
  
private:
  LEDSegment leds_;
  int index_;
  int delta_;
  int size_;
  CRGB color_;
};

#endif
