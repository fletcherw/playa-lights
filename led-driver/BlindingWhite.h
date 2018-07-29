#ifndef BLINDING_WHITE_H
#define BLINDING_WHITE_H

#include "Pattern.h"

class BlindingWhite : public Pattern {
public:
  BlindingWhite(CRGB *leds) :
    leds_(leds)
  {}

  void blit();
  int updateInterval();
  
private:
  CRGB *leds_;
};

#endif
