#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "LEDSegment.h"
#include "Pattern.h"

class Heartbeat : public Pattern {
public:
  Heartbeat(LEDSegment leds);

  void blit();

private:
  float bump_(float center, float width);

  float phase_;
  LEDSegment leds_;
  CRGB color_;
};

#endif
