#ifndef PATTERN_H
#define PATTERN_H

#include "FastLED.h"

class Pattern {
public:
  Pattern() {}
  virtual ~Pattern() {}
  virtual void blit() = 0;
};

#endif

