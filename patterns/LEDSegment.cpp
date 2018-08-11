#include "assert.h"

#include "LEDSegment.h"

int LEDSegment::length() {
  return abs(end_ - start_) + 1;
}

CRGB& LEDSegment::operator[](int index) {
  assert(0 <= index && index < length()); 
  if (start_ > end_) index *= -1;
  return array_[start_ + index];
}
