#ifndef MOVING_MOUND_H
#define MOVING_MOUND_H

#include "LEDSegment.h"
#include "Pattern.h"
//#include "led_utilities.h"

class MovingMound : public Pattern {
public:
  MovingMound(LEDSegment leds, int width, int index_init =0);
  ~MovingMound();
  void blit();
  int  getUpdateInterval();
  void setUpdateInterval(int updateInterval);
  void setColor(CRGB c);
  void setOverwrite(int is_overwrite);
  void setBounce(int is_bounce); 
  
private:
  LEDSegment leds_;
  CRGB color_;
  int index_;
  int delta_;
  int width_;
  bool is_overwrite_;
  bool is_bounce_;
  int* moundArray_; 
  void updateLED(int index, CRGB c);
  void createMoundArray(); 
  void clearLED(int index, CRGB c);
};

#endif

