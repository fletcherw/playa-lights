#include "MovingMound.h"

// Constructor Function 
MovingMound::MovingMound(LEDSegment leds, int width, int index_init =0) : 
	leds_(leds),
    color_(CRGB::Red),
    index_(index_init),
    width_(width)
{
  delta_ = 1; // Start moving forwards
  is_overwrite_ = false;
  is_bounce_ = false; 
  moundArray_ = new int[leds_.length()];
  
  // Make array with mound intensity values 
  moundArray_[width] = 1; 
  for (int i = 1; i <= width; i++){
    moundArray_[width+i ] = pow(4, i);
    moundArray_[width-i ] = pow(4, i);
  }
}

// LED Update Function
void MovingMound::blit() {
	
	// Erase old LEDs
	// Draw the trailing and leading LEDs
	clearLED(index_,color_/moundArray_[width_]);
  for (int k = 1; k <= width_; k++) {
	  
	// Draw trailing LEDs
	int led_index = index_ - k; 
	if (led_index > 0) {
		clearLED(led_index, color_/moundArray_[width_-k]);
	} else if (!is_bounce_) {
		clearLED(led_index+leds_.length()-1, color_/moundArray_[width_-k]);
	}
	
	// Draw leading LEDs
	led_index = index_ + k;   
	if (led_index < leds_.length()) { 
		clearLED(led_index, color_/moundArray_[width_-k]);
	} else if (!is_bounce_) {
		clearLED(led_index-leds_.length(),color_/moundArray_[width_-k]);
	}
  }

  index_ += delta_; // Increment LED

  // Check bounce/loop conditions
  if (index_ >= leds_.length()) {
    if (is_bounce_){
      delta_ = -1;
      index_ = leds_.length() - 2;
    }else{
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
  for (int k = 1; k <= width_; k++) {
	  
	// Draw trailing LEDs
    int led_index = index_ - k; 
    if (led_index > 0) {
		updateLED(led_index, color_/moundArray_[width_-k]);
    } else if (!is_bounce_) {
		updateLED(led_index+leds_.length()-1, color_/moundArray_[width_-k]);
    }
	
	// Draw leading LEDs
    led_index = index_ + k;   
    if (led_index < leds_.length()) { 
		updateLED(led_index, color_/moundArray_[width_-k]);
    } else if (!is_bounce_) {
		updateLED(led_index-leds_.length(),color_/moundArray_[width_-k]);
    }
  }
}


int MovingMound::getUpdateInterval() {
  return updateInterval_;
}

void MovingMound::setUpdateInterval(int updateInterval){
  updateInterval_ = updateInterval; 
}

void MovingMound::setColor(CRGB c) {
  color_ = c;
  blit();
}

void MovingMound::setOverwrite(int is_overwrite){
  // Check that the overwrite input is valid bool
  if (is_overwrite == 1 || is_overwrite ==0){
    is_overwrite_ = is_overwrite;
  }
}

void MovingMound::setBounce(int is_bounce){
  // Check that the is_bounce input is valid bool
  if (is_bounce == 1 || is_bounce ==0){
    is_bounce_ = is_bounce;
  }
}

void MovingMound::updateLED(int index, CRGB c){
  if (is_overwrite_){
    leds_[index] = c; 
  } else {
    leds_[index] += c; 
  }
}

void MovingMound::clearLED(int index, CRGB c){
	leds_[index] -= c; 
}


MovingMound::~MovingMound() {
  delete moundArray_;
}

