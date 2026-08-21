#ifndef GLITTER_H
#define GLITTER_H

#include "LEDSegment.h"
#include "Pattern.h"

class Glitter : public Pattern {
public:
  Glitter(LEDSegment leds);
  ~Glitter();

  void blit();
  void setColor(CRGB c);
  CRGB getColor();

private:
  struct PixelState {
    float phase;        // 0..1, this pixel's position in its own breathing cycle
    float cyclesPerSec;  // this pixel's breathing speed
    int8_t hueShift;
    int8_t satShift;
  };

  // Small deterministic integer hash (no external entropy), so each pixel
  // index always gets the same phase/speed/color-shift on every activation.
  static uint32_t hash_(uint32_t x);
  static float hashFloat_(uint32_t x); // 0..1

  LEDSegment leds_;
  CRGB color_;
  PixelState* states_;
  long lastBlit_;
};

#endif
