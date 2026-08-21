#include "Glitter.h"

namespace {
  const float kMinCyclesPerSec = 1.0 / 4.0;  // slowest breathing: 4s per cycle
  const float kMaxCyclesPerSec = 1.0 / 1.5;  // fastest breathing: 1.5s per cycle
  const int kMaxHueShift = 10;               // out of 255 (~14 degrees), so it stays "slight"
  const int kMaxSatShift = 20;               // out of 255
}

uint32_t Glitter::hash_(uint32_t x) {
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}

float Glitter::hashFloat_(uint32_t x) {
  return (hash_(x) & 0xFFFFFF) / float(0x1000000);
}

Glitter::Glitter(LEDSegment leds) :
  leds_(leds),
  color_(CRGB::White)
{
  lastBlit_ = millis();
  states_ = new PixelState[leds_.length()];
  for (int i = 0; i < leds_.length(); i++) {
    states_[i].phase = hashFloat_(i * 3 + 1);
    float speedFraction = hashFloat_(i * 3 + 2);
    states_[i].cyclesPerSec =
        kMinCyclesPerSec + speedFraction * (kMaxCyclesPerSec - kMinCyclesPerSec);
    states_[i].hueShift = int(hashFloat_(i * 3 + 100) * 2 * kMaxHueShift) - kMaxHueShift;
    states_[i].satShift = int(hashFloat_(i * 3 + 200) * 2 * kMaxSatShift) - kMaxSatShift;
  }
  setUpdateInterval(20);
}

Glitter::~Glitter() {
  delete[] states_;
}

void Glitter::setColor(CRGB c) {
  color_ = c;
}

CRGB Glitter::getColor() {
  return color_;
}

void Glitter::blit() {
  long now = millis();
  float deltaSec = (now - lastBlit_) / 1000.0;
  lastBlit_ = now;

  // Shift in HSV space, not RGB: a fixed RGB delta reads very differently
  // depending on the base color (e.g. blue's channel barely affects
  // perceived brightness compared to red/green), but a fixed hue/saturation
  // shift looks like "the same amount" of variation regardless of hue.
  CHSV baseHSV = rgb2hsv_approximate(color_);

  for (int i = 0; i < leds_.length(); i++) {
    PixelState& state = states_[i];
    state.phase += state.cyclesPerSec * deltaSec;
    if (state.phase >= 1.0) state.phase -= int(state.phase);

    float brightnessFraction = 0.5 + 0.5 * sin(state.phase * 2 * PI);

    CHSV pixelHSV(
        baseHSV.hue + state.hueShift,
        constrain(int(baseHSV.sat) + state.satShift, 0, 255),
        baseHSV.val);
    CRGB pixelColor;
    hsv2rgb_rainbow(pixelHSV, pixelColor);
    pixelColor %= int(255 * brightnessFraction);
    leds_[i] = pixelColor;
  }
}
