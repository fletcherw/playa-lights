#ifndef PATTERN_H
#define PATTERN_H

#include "FastLED.h"

class Pattern {
public:
  virtual ~Pattern() {}
  virtual void blit() = 0;

  virtual void setUpdateInterval(int interval) {
    updateInterval_ = interval;
  }

  virtual int getUpdateInterval() {
    return updateInterval_;
  }
  
  virtual void setColor(CRGB c) {}

protected:
  int updateInterval_ = 50;
};

#endif

