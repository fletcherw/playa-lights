#include "FastLED.h"

#define LED_DATA_PIN 12
#define COLOR_ORDER GRB
#define NUM_LEDS 39
#define LED_TYPE WS2812B

#include "src/patterns/Pattern.h"
#include "src/patterns/SpinningRainbow.h"
#include "src/patterns/PingPong.h"
#include "src/patterns/Metronome.h"
#include "src/patterns/Solid.h"
#include "src/patterns/Sparkle.h"
#include "src/patterns/Fire.h"
#include "src/patterns/Pulse.h"

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

// array used to display patterns
CRGB leds[NUM_LEDS];
int activePatternIndex;
Pattern* activePattern;
CRGB activeColor;
int blitInterval;
int brightness;
bool paused;
long lastBlit;

#define BUF_SIZE 256
// buffer for incoming commands
size_t messageLength;
size_t bytesRead;
bool inMessage;
char message[BUF_SIZE];

void clearLeds() {
   for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
   FastLED.show();
}

void setup() {
  brightness = 255;
  paused = false;
  activePatternIndex = 5;
  activeColor = CRGB::White;
  
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2400); 
  FastLED.setBrightness(brightness);
  clearLeds();

  activePattern = new Pulse(leds);
  lastBlit = millis();
  blitInterval = activePattern->updateInterval();

  Serial.begin(9600);
  Serial1.begin(9600);
  messageLength = 0;
  bytesRead = 0;
  inMessage = false;
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

void handleCommand() {
  switch(message[0]) {
    case 'B': {
      brightness = message[1];
      FastLED.setBrightness(brightness);
      break;
    }
    case 'C': {
      activeColor = CRGB(message[1], message[2], message[3]);
      activePattern->setColor(activeColor);
      FastLED.show();
      break;
    }
    case 'G': {
      Serial1.write('\6');
      Serial1.write(paused ? '0' : '1');
      Serial1.write(128);
      Serial1.write(activeColor.red);
      Serial1.write(activeColor.green);
      Serial1.write(activeColor.blue);
      Serial1.write(activePatternIndex);
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
        if (message[1] == activePatternIndex) break;
        if (activePattern != nullptr) {
          delete activePattern;
          clearLeds();
        }
        activePatternIndex = message[1];
        switch ((pattern_type) message[1]) {
          case SPINNING_RAINBOW:
            activePattern = new SpinningRainbow(leds);
            break;
          case PING_PONG:
            activePattern = new PingPong(leds);
            break;
          case METRONOME:
            activePattern = new Metronome(leds);
            break;
          case SOLID:
            activePattern = new Solid(leds);
            break;
          case SPARKLE:
            activePattern = new Sparkle(leds);
            break;
          case FIRE:
            activePattern = new Fire(leds);
            break;
          case PULSE:
            activePattern = new Pulse(leds);
            break;
        }
        activePattern->setColor(activeColor);
        activePattern->blit();
        lastBlit = millis();
        FastLED.show();
        blitInterval = activePattern->updateInterval();
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

void loop() 
{
  long time = millis();
  if (activePattern != nullptr && !paused &&
      blitInterval != -1 && (time - lastBlit) >= blitInterval) {
    activePattern->blit();
    FastLED.show();
    lastBlit = time;
  }
  receiveBluetooth();
  FastLED.delay(10);
}

//void scalePingPong() {
//  idx += ppDelta;
//  if (idx >= NUM_LEDS) {
//    ppDelta *= -1;
//    idx = NUM_LEDS - 1 + ppDelta;
//  } else if (idx < 0) {
//    ppDelta *= -1;
//    idx = ppDelta;
//  }
//
//  for (int i = 0; i < NUM_LEDS; i++) {
//    double fraction;
//    if (i <= idx) {
//      fraction = double(i) / idx;
//    } else {
//      fraction = double(NUM_LEDS - i) / (NUM_LEDS - idx);
//    }    
//    leds[i].r = 40;
//    leds[i].b = 120;
//    leds[i] %= (int) (fraction * 100);
//  }
//
//  FastLED.show();
//  delay(50);
//}
//
//void sinCrawl() {
//  th += 0.1;
//  double ps = 0.3;
//  double maxBright = 140;
//  for (int i = 0; i < NUM_LEDS; i++) {
//    double value = sin(th + ps * (i - (NUM_LEDS / 2)));
//    double mapped = (value * value * value * value) * maxBright;
//    leds[i].g = 0;
//    leds[i].b = 0;
//    if (value > 0) {
//      leds[i].b = (int) abs(mapped);
//    } else {
//      leds[i].g = (int) abs(mapped);
//    }
//  }
//  FastLED.show();
//  delay(50);
//}


