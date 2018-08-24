#include "Fire.h"

int sparking = 55;

Fire::Fire(LEDSegment leds, int cooling) : leds_(leds), cooling_(cooling)
{
  updateInterval_ = 55;
  heat_ = new byte[leds_.length()]; 
}

Fire::~Fire() {
  delete heat_;
}

void Fire::blit() {
  for (int i = 0; i < leds_.length(); i++) {
    int cooldown = random(0, ((cooling_ * 10) / 12) + 2);
   
    if (cooldown > heat_[i]) {
      heat_[i] = 0;
    } else {
      heat_[i] = heat_[i] - cooldown;
    }
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for (int k = leds_.length() - 1; k >= 2; k--) {
    heat_[k] = (heat_[k - 1] + 2 * heat_[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if (random(255) < sparking) {
    int index = random(3);
    heat_[index] = heat_[index] + random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for (int j = 0; j < leds_.length(); j++) {
    leds_[j] = getPixelHeatColor_(j, heat_[j]);
  }
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
