#ifndef MOVING_MOUND_H
#define MOVING_MOUND_H

#include "LEDSegment.h"
#include "Pattern.h"
//#include "led_utilities.h"

class MovingMound : public Pattern {
public:
  MovingMound(LEDSegment leds, int radius, int indexInit = 0);
  ~MovingMound();
  void blit();
  void setColor(CRGB c);
  void setOverwrite(bool overwrite);
  void setBounce(bool bounce);

private:
  LEDSegment leds_;
  CRGB color_;
  int index_;
  int delta_;
  int radius_;
  bool overwrite_;
  bool bounce_;
  uint8_t* moundArray_;

  void updateLED(int index, CRGB c);
  void createMoundArray();
  void clearLED(int index, CRGB c);
};

#endif
