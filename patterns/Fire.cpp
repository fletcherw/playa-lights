#include "Fire.h"

int cooling = 15;
int sparking = 55;

void Fire::blit() {
  for (int i = 0; i < 24; i++) {
    int cooldown = random(0, ((15 * 10) / 12) + 2);
   
    if (cooldown > heat[i]) {
      heat[i] = 0;
    } else {
      heat[i] = heat[i] - cooldown;
    }
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for (int k = 11; k >= 2; k--) {
    heat[k] = (heat[k - 1] + 2 * heat[k - 2]) / 3;
  }
  
  for (int k = 12; k < 22; k++) {
    heat[k] = (heat[k + 1] + 2 * heat[k + 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if (random(255) < sparking) {
    int y = random(4);
    int index;
    if (y <= 1) index = y;
    else index = 24 - y + 1;
    
    heat[index] = heat[index] + random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for (int j = 0; j < 24; j++) {
    leds_[j] = getPixelHeatColor_(j, heat[j]);
  }
}

int Fire::updateInterval() {
  return 55;
}

CRGB Fire::getPixelHeatColor_(int pixel, byte temperature) {
  // figure out which third of the spectrum we're in:
  int lowStart = 0;
  int midStart = 100;
  int highStart = 200;
  
  if (temperature > highStart) {
    // hottest
    float heatramp = ((float) (temperature - highStart)) / (255 - highStart);
    return CHSV(35, 255 - (heatramp * 230) , 255);
  } else if (temperature > midStart) {
    float heatramp = ((float) (temperature - midStart)) / (highStart - midStart);
    // middle
    return CHSV(heatramp * 25 + 10, 255, 255);
  } else {
    float heatramp = ((float) (temperature - lowStart)) / (midStart - lowStart);
    // coolest
    return CHSV(10, 255, heatramp * 255);
  }
}
