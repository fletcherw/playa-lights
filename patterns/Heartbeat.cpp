#include "Heartbeat.h"

namespace {
  const float kCycleMillis = 60.0 * 1000.0 / 35.0;  // minutes per beat
  const float kBaselineFraction = 0.18; // never fully dark
  const float kLubCenter = 0.12;
  const float kLubWidth = 0.09;
  const float kDubCenter = 0.30;
  const float kDubWidth = 0.07;
  const float kDubAmplitude = 0.55;    // second beat is quieter than the first
  const float kSharpness = 1.2;        // lower = slower fade up/down per beat
}

Heartbeat::Heartbeat(LEDSegment leds) :
  phase_(0.0),
  leds_(leds),
  color_(CRGB(255, 40, 0))
{
  setUpdateInterval(20);
}

void Heartbeat::blit() {
  phase_ += updateInterval_ / kCycleMillis;
  if (phase_ >= 1.0) phase_ -= 1.0;

  float lub = bump_(kLubCenter, kLubWidth);
  float dub = kDubAmplitude * bump_(kDubCenter, kDubWidth);
  float pulse = max(lub, dub);

  float brightnessFraction = kBaselineFraction + (1.0 - kBaselineFraction) * pulse;
  int scale = int(255 * brightnessFraction);

  for (int i = 0; i < leds_.length(); i++) {
    leds_[i] = color_;
    leds_[i] %= scale;
  }
}

float Heartbeat::bump_(float center, float width) {
  float d = (phase_ - center) / width;
  return exp(-d * d * kSharpness);
}
