#include "FastLED.h"

#define LED_DATA_PIN 12
#define COLOR_ORDER GRB
#define NUM_LEDS 39
#define LED_TYPE WS2812B

#include "src/patterns/Pattern.h"
#include "src/patterns/LEDSegment.h"

/* vvv KEEP IN SYNC vvv */
#include "src/patterns/SpinningRainbow.h"
#include "src/patterns/PingPong.h"
#include "src/patterns/Solid.h"
#include "src/patterns/Sparkle.h"
#include "src/patterns/Fire.h"
#include "src/patterns/Pulse.h"
#include "src/patterns/Meteor.h"

#define NUM_PATTERNS 7

enum pattern_type {
  SPINNING_RAINBOW,
  PING_PONG,
  SOLID,
  SPARKLE,
  FIRE,
  PULSE,
  METEOR
};
/* ^^^ KEEP IN SYNC ^^^ */

enum driver_mode {
  BAG = 'G',
  BIKE = 'K'
};

// array used to display patterns
CRGB leds[NUM_LEDS];

LEDSegment all = LEDSegment(leds, 0, 38);

LEDSegment packLeft = LEDSegment(leds, 0, 11);
LEDSegment packRight = LEDSegment(leds, 23, 12); // reversed
LEDSegment packHoop = LEDSegment(leds, 0, 23);

LEDSegment patchOne = LEDSegment(leds, 24, 28);
LEDSegment patchTwo = LEDSegment(leds, 33, 29); // reversed
LEDSegment patchThree = LEDSegment(leds, 34, 38);


// current pattern settings
pattern_type activePattern;
CRGB activeColor;
byte brightness;
bool paused;
driver_mode mode;

// data related to running patterns
#define MAX_ACTIVE_PATTERNS 5
int numActivePatternSegments;
Pattern* activePatternSegments[MAX_ACTIVE_PATTERNS];
long lastBlit[MAX_ACTIVE_PATTERNS];

// buffer for incoming commands
#define BUF_SIZE 128
size_t messageLength;
size_t bytesRead;
bool inMessage;
char message[BUF_SIZE];

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  messageLength = 0;
  bytesRead = 0;
  inMessage = false;

  activeColor = CRGB::White;
  brightness = 255;
  paused = false;
  mode = BAG;
  
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2400); 
  FastLED.setBrightness(brightness);
  clearLeds();

  setPattern(METEOR);
}

void loop() {
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
  }
  receiveBluetooth();
  FastLED.delay(10);
}

void setPattern(pattern_type p) {
  if (activePattern == p) return;
  activePattern = p;
  for (int i = 0; i < numActivePatternSegments; i++) {
    delete activePatternSegments[i];
  }
  switch (p) {
    case SPINNING_RAINBOW:
      activePatternSegments[0] = new SpinningRainbow(packHoop, 75);
      activePatternSegments[1] = new SpinningRainbow(patchOne, 20);
      activePatternSegments[2] = new SpinningRainbow(patchTwo, 20);
      activePatternSegments[3] = new SpinningRainbow(patchThree, 20);
      numActivePatternSegments = 4;
      break;
    case PING_PONG:
      activePatternSegments[0] = new PingPong(packHoop);
      activePatternSegments[1] = new PingPong(patchOne);
      activePatternSegments[2] = new PingPong(LEDSegment(leds, 29, 33));
      activePatternSegments[3] = new PingPong(patchThree);
      activePatternSegments[1]->setUpdateInterval(100);
      activePatternSegments[2]->setUpdateInterval(100);
      activePatternSegments[3]->setUpdateInterval(100);
      numActivePatternSegments = 4;
      break;
    case SOLID:
      activePatternSegments[0] = new Solid(all);
      numActivePatternSegments = 1;
      break;
    case SPARKLE:
      activePatternSegments[0] = new Sparkle(all);
      numActivePatternSegments = 1;
      break;
    case FIRE:
      activePatternSegments[0] = new Fire(packLeft);
      activePatternSegments[1] = new Fire(packRight);
      activePatternSegments[2] = new Pulse(patchOne);
      activePatternSegments[3] = new Pulse(patchTwo);
      activePatternSegments[4] = new Pulse(patchThree);
      activePatternSegments[2]->setColor(CRGB::Red);
      activePatternSegments[3]->setColor(CRGB::Red);
      activePatternSegments[4]->setColor(CRGB::Red);
      numActivePatternSegments = 5;
      break;
    case PULSE:
      activePatternSegments[0] = new Pulse(all);
      numActivePatternSegments = 1;
      break;
    case METEOR:
      activePatternSegments[0] = new Meteor(packHoop);
      numActivePatternSegments = 1;
      break;
  }
  long currentTime = millis();
  for (int i = 0; i < numActivePatternSegments; i++) {
    if (usesUserColor(p)) activePatternSegments[i]->setColor(activeColor);
    activePatternSegments[i]->blit();
    lastBlit[i] = currentTime;
  }  
}

void handleCommand() {
  switch(message[0]) {
    case 'B': {
      brightness = message[1];
      FastLED.setBrightness(brightness);
      break;
    }
    case 'C': {
      activeColor = CRGB(message[1], message[2], message[3]);
      for (int i = 0; i < numActivePatternSegments; i++) {
        activePatternSegments[i]->setColor(activeColor);
      }
      FastLED.show();
      break;
    }
    case 'G': {
      Serial1.write('\7');
      Serial1.write(paused ? '0' : '1');
      Serial1.write(mode);
      Serial1.write(brightness);
      Serial1.write(activeColor.red);
      Serial1.write(activeColor.green);
      Serial1.write(activeColor.blue);
      Serial1.write(activePattern);
      break;
    }
    case 'M': {
      mode = (driver_mode) message[1];
      break;
    }
    case 'P': {
      if (!checkLength(2)) break;
      if (message[1] == '0') {
        clearLeds();
        paused = true;
      } else if (message[1] == '1') {
        paused = false;
      } else {
        Serial.println("Unsupported 'P' code received");
        printMessage();
      }
      break;
    }
    case 'S': {
      if (!checkLength(2)) break;
      if (message[1] >= NUM_PATTERNS) {
        Serial.println("Unsupported pattern received for 'S'");
        printMessage();
      } else {
        pattern_type newPattern = (pattern_type) message[1];
        if (newPattern == activePattern) break;
        clearLeds();
        setPattern(newPattern);
        FastLED.show();
      }
      break;
    }
  }
}

void receiveBluetooth() {
  int commandsProcessed = 0;
  while (Serial1.available()) {
    char read = Serial1.read();
    if (!inMessage) {
      messageLength = read;
      Serial.println(messageLength);
      if (messageLength == 0) continue;
      inMessage = true;
      bytesRead = 0;
    } else {
      char received = read;
      message[bytesRead] = received;
      bytesRead++;
      if (bytesRead == messageLength) {
        printMessage();
        handleCommand();
        commandsProcessed++;
        if (commandsProcessed == 5) return;
        inMessage = false;
      }
    }
  }
}

bool usesUserColor(pattern_type p) {
  switch (p) {
    case PING_PONG:
    case SOLID:
    case SPARKLE:
    case PULSE:
    case METEOR:
      return true;
    case SPINNING_RAINBOW:
    case FIRE:
    default:
      return false;
  }
}

void clearLeds() {
   for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
   FastLED.show();
}

void printMessage() {
  Serial.print("Command: ");
  message[bytesRead] = '\0';
  Serial.println(message);
}

bool checkLength(int desiredLength) {
  if (desiredLength != bytesRead) {
    Serial.print("Incorrect message length, expected ");
    Serial.print(desiredLength);
    Serial.print(" received ");
    Serial.println(bytesRead);
    printMessage();
    return false;
  }
  return true;
}
