#include <string.h>

#include "FastLED.h"

#define LED_DATA_PIN 12
#define BUTTON_PIN 6
#define COLOR_ORDER GRB
#define NUM_LEDS 59
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
#include "src/patterns/Heartbeat.h"
#include "src/patterns/Glitter.h"

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
  HEARTBEAT,
  GLITTER,
  PATTERN_COUNT, // keep as second-to-last
  INVALID // keep as last
};

// indices must line up with pattern_type
const char* patternNames[PATTERN_COUNT] = {
  "Spinning Rainbow",
  "Ping Pong",
  "Solid",
  "Sparkle",
  "Fire",
  "Pulse",
  "Meteor",
  "Moving Mound",
  "Random Meteor",
  "Theater Chase",
  "Ripple",
  "Heartbeat",
  "Glitter"
};

// array used to display patterns
CRGB leds[NUM_LEDS];

// Bike Segments
LEDSegment bikeAll = LEDSegment(leds, 0, 58);


// current pattern settings
pattern_type activePattern = INVALID;
CRGB activeColor = CRGB::White;
byte brightness = 255;
bool paused = false;
bool randomSwitch = false;
long lastRandomSwitch;

// data related to running patterns
#define MAX_ACTIVE_PATTERNS 5
int numPatternSegments = 0;
Pattern* patternSegments[MAX_ACTIVE_PATTERNS];
long lastBlit[MAX_ACTIVE_PATTERNS];

// buffer for incoming commands
#define BUF_SIZE 128
#define MAX_MESSAGE_LENGTH 32
// printMessage() null-terminates at message[bytesRead], so the buffer needs
// room for the longest frame plus that terminator.
static_assert(MAX_MESSAGE_LENGTH < BUF_SIZE, "BUF_SIZE too small for MAX_MESSAGE_LENGTH");
size_t messageLength = 0;
size_t bytesRead = 0;
bool inMessage = false;
char message[BUF_SIZE];

// push button
int buttonState;
int lastButtonState = LOW;
unsigned long lastToggleTime;
unsigned long lastDebounceTime;
unsigned long debounceDelay = 1000;

void setup() {
  Serial.begin(9600);
  Serial.println("LED Driver is starting");
  Serial1.begin(9600);

  pinMode(BUTTON_PIN, INPUT);
  lastRandomSwitch = millis();
  lastToggleTime = millis();

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
        p->blit();
        lastBlit[i] = time;
      }
    }
  }

  if (randomSwitch && (time - lastRandomSwitch > RANDOM_SWITCH_INTERVAL)) {
    pattern_type next = activePattern;
    while (next == activePattern || next == RIPPLE) next = (pattern_type) random(PATTERN_COUNT);
    clearLeds();
    setPattern(next);
    lastRandomSwitch = time;
  }

  receiveBluetooth();
  checkButton();
  if (!paused) FastLED.delay(10);
  else delay(10);
}

void checkButton() {
  unsigned long time = millis();
  if (time - lastToggleTime < 5000) return;
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) lastDebounceTime = time;

  if ((time - lastDebounceTime) > debounceDelay) {
    lastToggleTime = time;
    if (reading != buttonState) {
      buttonState = reading;
      Serial.print("Switched button state to ");
      Serial.println(reading);
    }
    if (buttonState == HIGH) {
      // No action currently wired up to the button press.
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
  switch (p) {
    case SPINNING_RAINBOW:
      patternSegments[0] = new SpinningRainbow(bikeAll, 75);
      numPatternSegments = 1;
      break;
    case PING_PONG:
      patternSegments[0] = new PingPong(bikeAll);
      numPatternSegments = 1;
      break;
    case SOLID:
      patternSegments[0] = new Solid(bikeAll);
      numPatternSegments = 1;
      break;
    case SPARKLE:
      patternSegments[0] = new Sparkle(bikeAll);
      numPatternSegments = 1;
      break;
    case FIRE:
      patternSegments[0] = new Fire(bikeAll, 10);
      numPatternSegments = 1;
      break;
    case PULSE:
      patternSegments[0] = new Pulse(bikeAll);
      numPatternSegments = 1;
      break;
    case METEOR: {
      patternSegments[0] = new Meteor(bikeAll);
      numPatternSegments = 1;
      break;
    }
    case MOVING_MOUND: {
      MovingMound *m;
      m = new MovingMound(bikeAll, 9);
      m->setUpdateInterval(100);
      patternSegments[0] = m;

      m = new MovingMound(bikeAll, 9);
      m->setUpdateInterval(80);
      m->setColor(CRGB::Green);
      m->setBounce(true);
      patternSegments[1] = m;

      m = new MovingMound(bikeAll, 9);
      m->setUpdateInterval(60);
      m->setColor(CRGB::Blue);
      patternSegments[2] = m;

      numPatternSegments = 3;
      break;
    }
    case RANDOM_METEOR:
      patternSegments[0] = new RandomMeteor(bikeAll);
      numPatternSegments = 1;
      break;
    case THEATER_CHASE: {
      TheaterChase* tc = new TheaterChase(bikeAll);
      tc->setUpdateInterval(150);
      patternSegments[0] = tc;
      numPatternSegments = 1;
      break;
    }
    case RIPPLE:
      patternSegments[0] = new Ripple(bikeAll);
      numPatternSegments = 1;
      break;
    case HEARTBEAT:
      patternSegments[0] = new Heartbeat(bikeAll);
      numPatternSegments = 1;
      break;
    case GLITTER:
      patternSegments[0] = new Glitter(bikeAll);
      numPatternSegments = 1;
      break;
    default:
      numPatternSegments = 0;
      break;
  }
  long currentTime = millis();
  for (int i = 0; i < numPatternSegments; i++) {
    // Heartbeat has its own signature default color; don't stomp it with
    // whatever was last set on a different pattern. Instead, once it's
    // active, adopt its color as the new activeColor so 'G'/'S' responses
    // (and any later 'C' command) stay consistent with what's actually lit.
    if (usesUserColor(p) && p != HEARTBEAT) patternSegments[i]->setColor(activeColor);
    lastBlit[i] = currentTime;
  }
  if (p == HEARTBEAT) activeColor = patternSegments[0]->getColor();
}

void sendState() {
  Serial1.write((char) 0x8);
  Serial1.write('U');
  Serial1.write(paused ? '0' : '1');
  Serial1.write(randomSwitch ? '1' : '0');
  Serial1.write(brightness);
  Serial1.write(activeColor.red);
  Serial1.write(activeColor.green);
  Serial1.write(activeColor.blue);
  Serial1.write(activePattern);
}

void sendPatternList() {
  // response length is capped at 255 by the single-byte length prefix;
  // PATTERN_COUNT patterns with short names comfortably fit under that.
  int length = 2; // type char + count byte
  for (int i = 0; i < PATTERN_COUNT; i++) {
    length += 2 + strlen(patternNames[i]); // name length byte + name bytes + color-support byte
  }

  Serial1.write((char) length);
  Serial1.write('L');
  Serial1.write((char) PATTERN_COUNT);
  for (int i = 0; i < PATTERN_COUNT; i++) {
    Serial1.write((char) strlen(patternNames[i]));
    Serial1.write(patternNames[i]);
    Serial1.write(usesUserColor((pattern_type) i) ? '1' : '0');
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
    case 'L': {
      sendPatternList();
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
        if (newPattern != activePattern) {
          clearLeds();
          setPattern(newPattern);
          if (!paused) FastLED.show();
        }
        // report back the resulting state, since switching pattern can
        // change the active color (see Heartbeat's default in setPattern).
        sendState();
      }
      break;
    }
  }
}

void receiveBluetooth() {
  int commandsProcessed = 0;
  while (Serial1.available()) {
    // Serial1.read() returns an int in 0..255; storing it in a (signed) char
    // would turn a 0x80-or-higher byte into a negative length below.
    int incoming = Serial1.read();
    if (incoming < 0) break;
    if (!inMessage) {
      // Length prefix. Anything outside the valid range is line noise, so
      // drop it and keep looking for a plausible frame start.
      if (incoming <= 0 || incoming > MAX_MESSAGE_LENGTH) continue;
      messageLength = incoming;
      inMessage = true;
      bytesRead = 0;
      Serial.print("Incoming length: ");
      Serial.println(messageLength);
    } else {
      message[bytesRead] = (char) incoming;
      bytesRead++;
      if (bytesRead == messageLength) {
        printMessage();
        handleCommand();
        // Must clear inMessage before the batch check below, or the next call
        // resumes mid-frame with bytesRead already past messageLength and
        // never finds a frame boundary again.
        inMessage = false;
        commandsProcessed++;
        if (commandsProcessed == 5) return;
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
    case HEARTBEAT:
    case GLITTER:
      return true;
    default:
      return false;
  }
}

void clearLeds() {
  for (int i = 0; i < FastLED[0].size(); i++) {
    leds[i] = CRGB::Black;
  }
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
