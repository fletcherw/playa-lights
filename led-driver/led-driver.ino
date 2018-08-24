#include "FastLED.h"

#define LED_DATA_PIN 12
#define BUTTON_PIN 6
#define COLOR_ORDER GRB
#define NUM_BAG_LEDS 39
#define NUM_BIKE_LEDS 39
#define NUM_LEDS 39
#define LED_TYPE WS2812B
#define RANDOM_SWITCH_INTERVAL 60000

#include "src/patterns/Pattern.h"
#include "src/patterns/LEDSegment.h"

#include "src/patterns/SpinningRainbow.h"
#include "src/patterns/PingPong.h"
#include "src/patterns/Solid.h"
#include "src/patterns/Sparkle.h"
#include "src/patterns/Fire.h"
#include "src/patterns/Pulse.h"
#include "src/patterns/Meteor.h"
#include "src/patterns/MovingMound.h"
#include "src/patterns/RandomMeteor.h"
#include "src/patterns/TheaterChase.h"
#include "src/patterns/Ripple.h"

enum pattern_type {
  SPINNING_RAINBOW,
  PING_PONG,
  SOLID,
  SPARKLE,
  FIRE,
  PULSE,
  METEOR,
  MOVING_MOUND,
  RANDOM_METEOR,
  THEATER_CHASE,
  RIPPLE,
  PATTERN_COUNT, // keep as second-to-last
  INVALID // keep as last
};

enum driver_mode {
  BAG = 'G',
  BIKE = 'K'
};

// array used to display patterns
CRGB leds[NUM_BIKE_LEDS];

// Bag Segments
LEDSegment all = LEDSegment(leds, 0, 38);

LEDSegment packLeft = LEDSegment(leds, 0, 11);
LEDSegment packRight = LEDSegment(leds, 23, 12); // reversed
LEDSegment packHoop = LEDSegment(leds, 0, 23);

LEDSegment patch = LEDSegment(leds, 24, 38);
LEDSegment patchOne = LEDSegment(leds, 24, 28);
LEDSegment patchTwo = LEDSegment(leds, 33, 29); // reversed
LEDSegment patchThree = LEDSegment(leds, 34, 38);

LEDSegment patchOneTwo = LEDSegment(leds, 24, 33);
LEDSegment patchTwoThree = LEDSegment(leds, 29, 38);

 
// current pattern settings
pattern_type activePattern = INVALID;
CRGB activeColor = CRGB::White;
byte brightness = 255;
bool paused = false;
driver_mode mode = BAG;
bool randomSwitch = true;
long lastRandomSwitch;

// data related to running patterns
#define MAX_ACTIVE_PATTERNS 5
int numPatternSegments = 0;
Pattern* patternSegments[MAX_ACTIVE_PATTERNS];
long lastBlit[MAX_ACTIVE_PATTERNS];

// buffer for incoming commands
#define BUF_SIZE 128
size_t messageLength = 0;
size_t bytesRead = 0;
bool inMessage = false;
char message[BUF_SIZE];

// push button
int buttonState;
int lastButtonState = LOW;
unsigned long lastDebounceTime;
unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(9600);
  Serial.println("LED Driver is starting");
  Serial1.begin(9600);

  pinMode(BUTTON_PIN, INPUT);
  lastRandomSwitch = millis();

  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2400);
  FastLED.setBrightness(brightness);
  clearLeds();

  setPattern(SPINNING_RAINBOW);
  Serial.println("LED Driver has finished setup");
}

void loop() {
  long time = millis();
  if (numPatternSegments != 0 && !paused) {
    for (int i = 0; i < numPatternSegments; i++) {
      Pattern* p = patternSegments[i];
      int interval = p->getUpdateInterval();
      if (interval != -1 && (time - lastBlit[i]) >= interval) {
        if (activePattern == RANDOM_METEOR && i == 1) {
          p->setColor(patternSegments[0]->getColor());
        }
        p->blit();
        lastBlit[i] = time;
      }
    }
  }

  if (randomSwitch && (time - lastRandomSwitch > RANDOM_SWITCH_INTERVAL)) {
    pattern_type next = activePattern;
    while (next == activePattern) next = (pattern_type) random(PATTERN_COUNT);
    setPattern(next);
    lastRandomSwitch = time;
  }

  receiveBluetooth();
  checkButton();
  if (!paused) FastLED.delay(10);
  else delay(10);
}

void checkButton() {
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) lastDebounceTime = millis();

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) buttonState = reading;
    if (buttonState == HIGH) {
      paused = !paused;
      if (paused) clearLeds();
      }
  }

  lastButtonState = reading;
}

void setPattern(pattern_type p) {
  if (activePattern == p) return;
  activePattern = p;
  for (int i = 0; i < numPatternSegments; i++) {
    delete patternSegments[i];
  }
  if (mode == BAG) {
    switch (p) {
      case SPINNING_RAINBOW:
        patternSegments[0] = new SpinningRainbow(packHoop, 75);
        patternSegments[1] = new SpinningRainbow(patchOne, 20);
        patternSegments[2] = new SpinningRainbow(patchTwo, 20);
        patternSegments[3] = new SpinningRainbow(patchThree, 20);
        numPatternSegments = 4;
        break;
      case PING_PONG:
        patternSegments[0] = new PingPong(packHoop);
        patternSegments[1] = new PingPong(patchOne);
        patternSegments[2] = new PingPong(LEDSegment(leds, 29, 33));
        patternSegments[3] = new PingPong(patchThree);
        patternSegments[1]->setUpdateInterval(100);
        patternSegments[2]->setUpdateInterval(100);
        patternSegments[3]->setUpdateInterval(100);
        numPatternSegments = 4;
        break;
      case SOLID:
        patternSegments[0] = new Solid(all);
        numPatternSegments = 1;
        break;
      case SPARKLE:
        patternSegments[0] = new Sparkle(all);
        numPatternSegments = 1;
        break;
      case FIRE:
        patternSegments[0] = new Fire(packLeft);
        patternSegments[1] = new Fire(packRight);
        patternSegments[2] = new Pulse(patch);
        patternSegments[2]->setColor(CRGB::Red);
        numPatternSegments = 3;
        break;
      case PULSE:
        patternSegments[0] = new Pulse(all);
        numPatternSegments = 1;
        break;
      case METEOR: {
        Meteor *m;
        m = new Meteor(packHoop, 5);
        patternSegments[0] = m;

        MovingMound *mm;
        mm = new MovingMound(patchOneTwo, 3);
        mm->setBounce(true);
        mm->setUpdateInterval(100);
        patternSegments[1] = mm;

        mm = new MovingMound(patchTwoThree, 3);
        mm->setBounce(true);
        mm->setUpdateInterval(100);
        patternSegments[2] = mm;

        numPatternSegments = 3;
        break;
      }
      case MOVING_MOUND: {
        MovingMound *m;
        m = new MovingMound(packHoop, 7);
        m->setUpdateInterval(100);
        patternSegments[0] = m;

        m = new MovingMound(packHoop, 7);
        m->setUpdateInterval(80);
        m->setColor(CRGB::Green);
        m->setBounce(true);
        patternSegments[1] = m;

        m = new MovingMound(packHoop, 7);
        m->setUpdateInterval(60);
        m->setColor(CRGB::Blue);
        patternSegments[2] = m;

        numPatternSegments = 3;
        break;
      }
      case RANDOM_METEOR:
        patternSegments[0] = new RandomMeteor(packHoop, 4);
        patternSegments[1] = new Pulse(patch);
        numPatternSegments = 2;
        break;
      case THEATER_CHASE:
        patternSegments[0] = new TheaterChase(packHoop);
        patternSegments[1] = new Pulse(patch);
        numPatternSegments = 2;
        break;
      case RIPPLE:
        patternSegments[0] = new Ripple(packHoop);
        numPatternSegments = 1;
        break;
      default:
        numPatternSegments = 0;
        break;
    }
  } else if (mode == BIKE) {
    switch (p) {
      case SPINNING_RAINBOW:
        patternSegments[0] = new SpinningRainbow(packHoop, 75);
        patternSegments[1] = new SpinningRainbow(patchOne, 20);
        patternSegments[2] = new SpinningRainbow(patchTwo, 20);
        patternSegments[3] = new SpinningRainbow(patchThree, 20);
        numPatternSegments = 4;
        break;
      case PING_PONG:
        patternSegments[0] = new PingPong(packHoop);
        patternSegments[1] = new PingPong(patchOne);
        patternSegments[2] = new PingPong(LEDSegment(leds, 29, 33));
        patternSegments[3] = new PingPong(patchThree);
        patternSegments[1]->setUpdateInterval(100);
        patternSegments[2]->setUpdateInterval(100);
        patternSegments[3]->setUpdateInterval(100);
        numPatternSegments = 4;
        break;
      case SOLID:
        patternSegments[0] = new Solid(all);
        numPatternSegments = 1;
        break;
      case SPARKLE:
        patternSegments[0] = new Sparkle(all);
        numPatternSegments = 1;
        break;
      case FIRE:
        patternSegments[0] = new Fire(packLeft);
        patternSegments[1] = new Fire(packRight);
        patternSegments[2] = new Pulse(patch);
        patternSegments[2]->setColor(CRGB::Red);
        numPatternSegments = 3;
        break;
      case PULSE:
        patternSegments[0] = new Pulse(all);
        numPatternSegments = 1;
        break;
      case METEOR: {
        patternSegments[0] = new Meteor(packHoop);
        MovingMound *m;
        m = new MovingMound(patchOneTwo, 3);
        m->setBounce(true);
        m->setUpdateInterval(100);
        patternSegments[1] = m;

        m = new MovingMound(patchTwoThree, 3);
        m->setBounce(true);
        m->setUpdateInterval(100);
        patternSegments[2] = m;

        numPatternSegments = 3;
        break;
      }
      case MOVING_MOUND: {
        MovingMound *m;
        m = new MovingMound(packHoop, 7);
        m->setUpdateInterval(100);
        patternSegments[0] = m;

        m = new MovingMound(packHoop, 7);
        m->setUpdateInterval(80);
        m->setColor(CRGB::Green);
        m->setBounce(true);
        patternSegments[1] = m;

        m = new MovingMound(packHoop, 7);
        m->setUpdateInterval(60);
        m->setColor(CRGB::Blue);
        patternSegments[2] = m;

        numPatternSegments = 3;
        break;
      }
      case RANDOM_METEOR:
        patternSegments[0] = new RandomMeteor(packHoop);
        patternSegments[1] = new Pulse(patch);
        numPatternSegments = 2;
        break;
      case THEATER_CHASE:
        patternSegments[0] = new TheaterChase(packHoop);
        patternSegments[1] = new Pulse(patch);
        numPatternSegments = 2;
        break;
      case RIPPLE:
        patternSegments[0] = new Ripple(packHoop);
        numPatternSegments = 1;
        break;
      default:
        numPatternSegments = 0;
        break;
    }
  }
  long currentTime = millis();
  if (activePattern == RANDOM_METEOR) {
    patternSegments[1]->setColor(patternSegments[0]->getColor());
  }
  for (int i = 0; i < numPatternSegments; i++) {
    if (usesUserColor(p)) patternSegments[i]->setColor(activeColor);
    lastBlit[i] = currentTime;
  }
}

void sendState() {
  Serial1.write((char) 0x8);
  Serial1.write(paused ? '0' : '1');
  Serial1.write(mode);
  Serial1.write(randomSwitch ? '1' : '0');
  Serial1.write(brightness);
  Serial1.write(activeColor.red);
  Serial1.write(activeColor.green);
  Serial1.write(activeColor.blue);
  Serial1.write(activePattern);
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
      for (int i = 0; i < numPatternSegments; i++) {
        patternSegments[i]->setColor(activeColor);
      }
      FastLED.show();
      break;
    }
    case 'G': {
      sendState();
      break;
    }
    case 'M': {
      driver_mode newMode = (driver_mode) message[1];
      if (mode == newMode) break;
      mode = newMode;

      if (mode == BAG) {
        FastLED[0].setLeds(leds, NUM_BAG_LEDS);
      } else if (mode == BIKE) {
        FastLED[0].setLeds(leds, NUM_BIKE_LEDS);
      }

      // force reinitialization of the pattern so that we will switch to bike or bag appropriate version;
      pattern_type lastPattern = activePattern;
      setPattern(INVALID);
      setPattern(lastPattern);
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
    case 'R': {
      if (!checkLength(2)) break;
      if (message[1] == '0') {
        randomSwitch = false;
      } else if (message[1] == '1') {
        randomSwitch = true;
        lastRandomSwitch = millis();
      } else {
        Serial.println("Unsupported 'R' code received");
        printMessage();
      }
      break;
    }
    case 'S': {
      if (!checkLength(2)) break;
      if (message[1] >= PATTERN_COUNT) {
        Serial.println("Unsupported pattern received for 'S'");
        printMessage();
      } else {
        pattern_type newPattern = (pattern_type) message[1];
        if (newPattern == activePattern) break;
        clearLeds();
        setPattern(newPattern);
        if (!paused) FastLED.show();
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
      return true;
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
