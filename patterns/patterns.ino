#include "FastLED.h"

#define DATA_PIN    3        // what output is connected to the LEDs
#define COLOR_ORDER GRB
#define NUM_LEDS    60
#define LED_TYPE    WS2812B

// array used to display patterns
CRGB leds[NUM_LEDS];

// variables used for various patterns
double th;
int idx;
int pp_delta;

void setup() {
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  th = 0;
  idx = 0;
  pp_delta = 1; 
}

void loop() 
{
  spinning_rainbow();
}

void spinning_rainbow() {
  double delta = 0.1;
  double value_target = 80.0;
  double value_radius = 15.0;

  idx++;
  th += delta;
  if (th > 2 * PI) th = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    double hue = double(i) / NUM_LEDS * 255;
    double value = value_target + sin(th) * value_radius;
    leds[(i + idx) % NUM_LEDS] = CHSV(int(hue), 255, int(value));
  }
  FastLED.show();
  delay(50);
}

void blinding_white() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
  }
  FastLED.show();
}

void dull_pulse_yellow() {
  CRGB color = CRGB(204, 102, 0);
  int target_bright = 30;
  int radius = 30;
  double delta = 0.025;
  
  th += delta;
  if (th > 2 * PI) th = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
    leds[i] %= int(target_bright + abs(sin(th)) * radius);
  }
  FastLED.show();
  delay(20);
}

void scale(CRGB color) {
  int maxval = 100;
  for (int i = 0; i < NUM_LEDS; i++) {
    double fraction = double(i) / NUM_LEDS;
    leds[i] = color;
    color %= 128 * fraction; 
  }
  FastLED.show();
}

void ping_pong() {
  leds[idx] = CRGB::Black;
  idx += pp_delta;
  if (idx >= NUM_LEDS) {
    pp_delta *= -1;
    idx = NUM_LEDS - 1 + pp_delta;
  } else if (idx < 0) {
    pp_delta *= -1;
    idx = pp_delta;
  }
  
  leds[idx].r = 40;
  leds[idx].b = 120;
  FastLED.show();
  delay(50);
}

void scale_ping_pong() {
  idx += pp_delta;
  if (idx >= NUM_LEDS) {
    pp_delta *= -1;
    idx = NUM_LEDS - 1 + pp_delta;
  } else if (idx < 0) {
    pp_delta *= -1;
    idx = pp_delta;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    double fraction;
    if (i <= idx) {
      fraction = double(i) / idx;
    } else {
      fraction = double(NUM_LEDS - i) / (NUM_LEDS - idx);
    }    
    leds[i].r = 40;
    leds[i].b = 120;
    leds[i] %= (int) (fraction * 100);
  }

  FastLED.show();
  delay(50);
}

void sin_crawl() {
  th += 0.1;
  double ps = 0.3;
  double max_bright = 140;
  for (int i = 0; i < NUM_LEDS; i++) {
    double value = sin(th + ps * (i - (NUM_LEDS / 2)));
    double mapped = (value * value * value * value) * max_bright;
    leds[i].g = 0;
    leds[i].b = 0;
    if (value > 0) {
      leds[i].b = (int) abs(mapped);
    } else {
      leds[i].g = (int) abs(mapped);
    }
  }
  FastLED.show();
  delay(50);
}


