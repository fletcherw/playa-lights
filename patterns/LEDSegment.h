#ifndef LEDSEGMENT_H_
#define LEDSEGMENT_H_

#include "FastLED.h"

class LEDSegment {
public:
  LEDSegment(CRGB* array, int start, int end)
    : array_(array),
      start_(start),
      end_(end)
  {}

  CRGB& operator[](int index);
  int length();

private:
  CRGB* array_;
  int start_;
  int end_;
};

#endif
