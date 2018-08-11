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

#include "Pattern.h"
//#include "SpinningRainbow.h"
#include "PingPong.h"
#include "Metronome.h"
#include "Solid.h"
#include "Sparkle.h"
#include "Fire.h"
#include "Pulse.h"
#include "MovingMound.h"

/* KEEP IN SYNC */
#define NUM_PATTERNS 7

enum pattern_type {
  SPINNING_RAINBOW,
  PING_PONG,
  METRONOME,
  SOLID,
  SPARKLE,
  FIRE,
  PULSE
};
/* KEEP IN SYNC */

//*****************************************************
// Variables
//*****************************************************
// array used to display patterns
CRGB leds[NUM_LEDS];
int activePatternIndex;
//Pattern* activePattern;
//Pattern* activePattern2;
MovingMound* activePattern;
MovingMound* activePattern2;

CRGB activeColor;
int blitInterval;
int blitInterval2;
int brightness;
bool paused;
long lastBlit;
long lastBlit2;
int potValue; 
int buttonState = 0;         // variable for reading the pushbutton status

#define BUF_SIZE 256
// buffer for incoming commands
size_t messageLength;
size_t bytesRead;
bool inMessage;
char message[BUF_SIZE];




//*****************************************************
// Setup
//*****************************************************
void setup() {
  Serial.begin(115200);
  messageLength = 0;
  bytesRead = 0;
  inMessage = false;
  Serial.println("Hello LED Program");
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  brightness = 255;
  paused = false;
  activePatternIndex = 5;
  activeColor = CRGB::White;
  
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2400); 
  FastLED.setBrightness(brightness);
  clearLeds();

  // Shape agnostic functions
  //activePattern = new Pulse(leds,NUM_LEDS);
  // activePattern = new Solid(leds,NUM_LEDS);
  //activePattern = new PingPong(leds,NUM_LEDS);
  activePattern = new MovingMound(leds,NUM_LEDS,3,10);
  activePattern->setColor(CRGB::Blue);
  activePattern->setUpdateInterval(100);
  activePattern->setOverwrite(false); 
  activePattern2 = new MovingMound(leds,NUM_LEDS,3);
  activePattern2->setColor(CRGB::Red);
  activePattern2->setOverwrite(false); 
  //activePattern2->setUpdateInterval(50); 
  // Needs 39 LEDS
  //activePattern = new SpinningRainbow(leds);
  //activePattern = new Metronome(leds);
  //activePattern = new Sparkle(leds);
  //activePattern = new Fire(leds);
  
  lastBlit = millis();
  lastBlit2 = lastBlit;
  blitInterval = activePattern->getUpdateInterval();
  blitInterval2 = activePattern2->getUpdateInterval();
  // If no periodic update, initialize update 
  /*if (blitInterval == -1){
     activePattern->blit();
  }*/
  
  Serial.println("Hello LED Program2");
}

void printMessage() {
  Serial.print("Command: ");
  message[bytesRead] = '\0';
  Serial.println(message);
}

//*****************************************************
// Loop
//*****************************************************
void loop() 
{ 
  potValue = analogRead(POT_PIN); // read the state of the pushbutton value      
  buttonState = digitalRead(BUTTON_PIN); // read the state of the pushbutton value
  
  long time = millis();

  // Update pattern 1
  if (activePattern != nullptr && !paused &&
      blitInterval != -1 && (time - lastBlit) >= blitInterval) {
    activePattern->blit();
    FastLED.show();
    lastBlit = time;
  }

  // Update pattern 2
  if (activePattern2 != nullptr && !paused &&
      blitInterval2 != -1 && (time - lastBlit2) >= blitInterval2) {
    activePattern2->blit();
    FastLED.show();
    lastBlit2 = time;
    Serial.println(blitInterval2);
  }
  FastLED.delay(10);
}



//*****************************************************
// Functions
//*****************************************************
void clearLeds() {
   for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
   FastLED.show();
}



