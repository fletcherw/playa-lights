#include "Scale.h"

void Scale::blit() {
  for (int i = 0; i < num_leds_; i++) {
    double fraction = double(i) / num_leds_;
    leds_[i] = color_;
    leds_[i] %= 128 * fraction; 
  }
}
