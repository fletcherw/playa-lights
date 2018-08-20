#include "Sparkle.h"
Sparkle::Sparkle(LEDSegment leds) :
    leds_(leds),
    lastBlit_(millis())
  {
    states_ = new pixelState[leds_.length()];
    for (int i = 0; i < leds_.length(); i++) {
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

Sparkle::~Sparkle() {
  delete states_;
}



CRGB Sparkle::calculateColor_(const Sparkle::pixelState& state) {
  double lifeFraction = double(state.age) / state.lifetime;
  if (state.age < 0 || lifeFraction >= 1) return CRGB::Black;
  if (lifeFraction < 0.8) {
    double intervalFraction = lifeFraction / 0.8;
    return CHSV(28 * intervalFraction, 255, intervalFraction * 70 + 185);
  }
  if (lifeFraction > 0.8) {
    double intervalFraction = (lifeFraction - 0.8) / 0.2;
    return CHSV(28 * (1 - intervalFraction), 255, (1 - intervalFraction) * 255);
  }
}

void Sparkle::blit() {
  int delta = millis() - lastBlit_;
  lastBlit_ = millis();
  for (int i = 0; i < leds_.length(); i++) {
    states_[i].age += delta;
    if (states_[i].age > states_[i].lifetime) {
      states_[i].lifetime = random(1000, 6000);
      states_[i].age = random(-10000, -3000);
    }
    leds_[i] = calculateColor_(states_[i]);
  }
}
