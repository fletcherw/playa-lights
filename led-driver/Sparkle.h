#ifndef SPARKLE_H
#define SPARKLE_H

#include "Pattern.h"

class Sparkle : public Pattern {
public:
  Sparkle(CRGB *leds) :
    leds_(leds),
    lastBlit_(millis()),
    color_(CRGB::Red)
  {
    for (int i = 0; i < 39; i++) {
      states_[i].lifetime = random(1000, 6000);
      bool alive = random(0, 4) == 0;
      if (alive) {
        states_[i].age = 0;
      } else {
        states_[i].age = random(-10000, -3000);
      }
      leds_[i] = calculateColor_(states_[i]);
    }
  }

  void blit();
  int updateInterval();
  void setColor(CRGB c);
  
private:
  struct pixelState {
    int age;
    int lifetime;
  };
  
  CRGB calculateColor_(const pixelState& state);

  CRGB *leds_;
  pixelState states_[39];
  long lastBlit_;
  CRGB color_;
};

#endif
