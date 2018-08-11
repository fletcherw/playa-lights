#include <SoftwareSerial.h>
#include "FastLED.h"

#define LED_TYPE WS2801
#define COLOR_ORDER RGB
#define NUM_LEDS 20 // How many leds are in the strip?
#define DATA_PIN 11 // Data pin that led data will be written out over
#define CLOCK_PIN 12
#define POT_PIN  A4    // select the input pin for the potentiometer
#define BUTTON_PIN A5 // select the button pin 
#define LED_PIN 13

#include "src/Pattern.h"
#include "src/SpinningRainbow.h"
#include "src/PingPong.h"
#include "src/Metronome.h"
#include "src/Solid.h"
#include "src/Sparkle.h"
#include "src/Fire.h"
#include "src/Pulse.h"
#include "src/MovingMound.h"
#include "src/Meteor.h"

/* KEEP IN SYNC */
#define NUM_PATTERNS 9

enum pattern_type {
  SPINNING_RAINBOW,
  PING_PONG,
  METRONOME,
  SOLID,
  SPARKLE,
  FIRE,
  PULSE,
  MOVING_MOUND,
  METEOR
};
/* KEEP IN SYNC */


//*****************************************************
// Variable Definitions
//*****************************************************
// array used to display patterns
CRGB leds[NUM_LEDS];
LEDSegment all = LEDSegment(leds, 0, NUM_LEDS - 1);
LEDSegment middle = LEDSegment(leds, 5, 15);
//LEDSegment packLeft = LEDSegment(leds, 0, 11);
//LEDSegment packRight = LEDSegment(leds, 23, 12); // reversed
//LEDSegment packHoop = LEDSegment(leds, 0, 23);

//LEDSegment patchOne = LEDSegment(leds, 24, 28);
//LEDSegment patchTwo = LEDSegment(leds, 33, 29); // reversed
//LEDSegment patchThree = LEDSegment(leds, 34, 38);

int activePatternIndex;
CRGB activeColor;

#define MAX_ACTIVE_PATTERNS 5
Pattern* activePatternSegments[MAX_ACTIVE_PATTERNS];
long lastBlit[MAX_ACTIVE_PATTERNS];
int numActivePatternSegments;
int brightness;
bool paused;

int potValue;
int buttonState = 0;         // variable for reading the pushbutton status

bool colorChanged;



//*****************************************************
// Setup
//*****************************************************
void setup() {

  // Initialize Serial
  Serial.begin(115200);
  Serial.println("Hello LED Controller");

  // Initialize Pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);


  // Initialize LEDs
  brightness = 255;
  paused = false;
  colorChanged = false;
  activePatternIndex = 5;
  activeColor = CRGB::White;

  FastLED.addLeds<LED_TYPE, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2400);
  FastLED.setBrightness(brightness);
  clearLeds();
  activePatternIndex = 3;//METEOR;
  setPattern(activePatternIndex);
}
//*****************************************************
// Loop
//*****************************************************
void loop()
{
  potValue = analogRead(POT_PIN); // read the state of the pushbutton value
  //buttonState = digitalRead(BUTTON_PIN); // read the state of the pushbutton value
  pollButton(); // Check button state

  // Update pattern s
  long time = millis();
  if (numActivePatternSegments != 0 && !paused) {
    for (int i = 0; i < numActivePatternSegments; i++) {
      Pattern* p = activePatternSegments[i];
      int interval = p->getUpdateInterval();
      if (interval != -1 && (time - lastBlit[i]) >= interval) {
        p->blit();
        lastBlit[i] = time;
      }
    }
    FastLED.show();
    //Serial.println("test");
  }

  FastLED.delay(10);

  /*if (time>5000 && !colorChanged){
    activePatternSegments[0]->setColor(CRGB::Red);
    activePatternSegments[2]->setColor(CRGB::Red);
    colorChanged=true;
    }*/
}



//*****************************************************
// Functions
//*****************************************************
void setPattern(pattern_type p) {
  for (int i = 0; i < numActivePatternSegments; i++) {
    delete activePatternSegments[i];
  }
  clearLeds();
  switch (p) {
    case SPINNING_RAINBOW:
      activePatternSegments[0] = new SpinningRainbow(all, 10);
      //activePatternSegments[1] = new SpinningRainbow(patchOne, 20);
      //activePatternSegments[2] = new SpinningRainbow(patchTwo, 20);
      //activePatternSegments[3] = new SpinningRainbow(patchThree, 20);
      numActivePatternSegments = 1;
      break;
    case PING_PONG:
      activePatternSegments[0] = new PingPong(middle);
      numActivePatternSegments = 1;
      break;
    //case METRONOME:
    //  break;
    case SOLID:
      activePatternSegments[0] = new Solid(all);
      numActivePatternSegments = 1;
      break;
    case SPARKLE:
      activePatternSegments[0] = new Sparkle(all);
      numActivePatternSegments = 1;
      break;
    case FIRE:
      activePatternSegments[0] = new Fire(all);
      //activePatternSegments[1] = new Fire(packRight);
      numActivePatternSegments = 1;
      break;
    case PULSE:
      activePatternSegments[0] = new Pulse(all);
      numActivePatternSegments = 1;
      break;
    case MOVING_MOUND:
      activePatternSegments[0] = new MovingMound(all, 3, 0);
      activePatternSegments[0]->setColor(CRGB::Blue);
      activePatternSegments[0]->setUpdateInterval(100);
      static_cast <MovingMound*>(activePatternSegments[0])->setBounce(false);

      activePatternSegments[1] = new MovingMound(all, 2, 0);
      activePatternSegments[1]->setColor(CRGB::Red);
      activePatternSegments[1]->setUpdateInterval(50);
      static_cast <MovingMound*>(activePatternSegments[1])->setBounce(false);

      activePatternSegments[2] = new MovingMound(all, 1, 0);
      activePatternSegments[2]->setColor(CRGB::Green);
      activePatternSegments[2]->setUpdateInterval(50);
      static_cast <MovingMound*>(activePatternSegments[2])->setBounce(true);

      numActivePatternSegments = 3;
      break;
    case METEOR:
      activePatternSegments[0] = new Meteor(all);
      numActivePatternSegments = 1;
      break;
    default:
      numActivePatternSegments =0; 
      break; 
  }
  long currentTime = millis();
  for (int i = 0; i < numActivePatternSegments; i++) {
    //activePatternSegments[i]->setColor(activeColor);
    activePatternSegments[i]->blit();
    lastBlit[i] = currentTime;
  }
}


void clearLeds() {
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  FastLED.show();
}

// Button click with debouncing
void pollButton() {
  if ( !digitalRead(BUTTON_PIN) ) {
    delay(30);
    while (!digitalRead(BUTTON_PIN)) {
      delay(1);
    }
    activePatternIndex++;
    activePatternIndex %= NUM_PATTERNS;
    setPattern(activePatternIndex);
    Serial.print("Switched mode to ");
    Serial.println((int) activePatternIndex);
    
  }
}

