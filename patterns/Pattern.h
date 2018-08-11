#ifndef PATTERN_H
#define PATTERN_H

#include "FastLED.h"

class Pattern {
public:
  virtual ~Pattern() {}
  virtual void blit() = 0;
  virtual int updateInterval() = 0;
  
  virtual void setColor(CRGB c) {}
};

#endif

