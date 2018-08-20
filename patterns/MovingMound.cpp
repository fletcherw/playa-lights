#include "MovingMound.h"

inline int mod(int n, int d) {
  return n % d + (n < 0 ? d : 0);
}

MovingMound::MovingMound(LEDSegment leds, int radius, int indexInit) :
	leds_(leds),
    color_(CRGB::Red),
    index_(indexInit),
    delta_(1), // Start moving forwards
    radius_(radius),
    overwrite_(false),
    bounce_(false)
{
  moundArray_ = new uint8_t[leds_.length()];

  // Make array with mound intensity values
  moundArray_[radius_] = 1;
  for (int i = 1; i <= radius_; i++) {
    moundArray_[radius + i] = min(pow(2, i), 255);
    moundArray_[radius - i] = min(pow(2, i), 255);
  }
}

// LED Update Function
void MovingMound::blit() {

	// Erase old LEDs
	// Draw the trailing and leading LEDs
	clearLED(index_, color_ / moundArray_[radius_]);
  for (int k = 1; k <= radius_; k++) {
    // Clear trailing LEDs
    int led_index = index_ - k;
    if (led_index > 0) {
      clearLED(led_index, color_ / moundArray_[radius_ - k]);
    } else if (!bounce_) {
      clearLED(led_index + leds_.length() - 1, color_ / moundArray_[radius_ -k]);
    }

    // Clear leading LEDs
    led_index = index_ + k;
    if (led_index < leds_.length()) {
      clearLED(led_index, color_ / moundArray_[radius_ - k]);
    } else if (!bounce_) {
      clearLED(led_index - leds_.length(),color_ / moundArray_[radius_ - k]);
    }
  }

  index_ += delta_; // Move center of mound

  // Check bounce/loop conditions
  if (index_ >= leds_.length()) {
    if (bounce_) {
      delta_ = -1;
      index_ = leds_.length() - 2;
    } else {
      index_ = 0;
    }
  } else if (index_ < 0) {
    delta_ = 1;
    index_ = 1;
  }

  // Redraw LEDs

  // Set main LED
  leds_[index_] = color_;
  // Draw the trailing and leading LEDs
  for (int k = 1; k <= radius_; k++) {

    // Draw trailing LEDs
    int led_index = index_ - k;
    if (led_index > 0) {
      updateLED(led_index, color_ / moundArray_[radius_ - k]);
    } else if (!bounce_) {
      updateLED(led_index + leds_.length() - 1, color_ / moundArray_[radius_ - k]);
    }

    // Draw leading LEDs
    led_index = index_ + k;
    if (led_index < leds_.length()) {
		updateLED(led_index, color_ / moundArray_[radius_ - k]);
    } else if (!bounce_) {
		updateLED(led_index - leds_.length(), color_ / moundArray_[radius_ - k]);
    }
  }
}

void MovingMound::setColor(CRGB c) {
  color_ = c;
  blit();
}

void MovingMound::setOverwrite(bool overwrite) {
  overwrite_ = overwrite;
}

void MovingMound::setBounce(bool bounce) {
  bounce_ = bounce;
}

void MovingMound::updateLED(int index, CRGB c) {
  if (overwrite_) {
    leds_[index] = c;
  } else {
    leds_[index] += c;
  }
}

void MovingMound::clearLED(int index, CRGB c) {
	leds_[index] -= c;
}

MovingMound::~MovingMound() {
  delete moundArray_;
}
