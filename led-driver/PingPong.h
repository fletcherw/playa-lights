#ifndef PING_PONG_H
#define PING_PONG_H

#include "Pattern.h"

class PingPong : public Pattern {
public:
  PingPong(CRGB *leds, int num_leds) :
    leds_(leds),
    num_leds_(num_leds),
    index_(0),
    delta_(1)
  {}

  void blit();
  
private:
  CRGB *leds_;
  int num_leds_;
  int index_;
  int delta_;
};

#endif
