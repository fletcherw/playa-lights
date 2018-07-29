#include <SoftwareSerial.h>

#include "FastLED.h"

#define LED_DATA_PIN 12
#define COLOR_ORDER GRB
#define NUM_LEDS 39
#define LED_TYPE WS2812B

#include "Pattern.h"
#include "SpinningRainbow.h"
#include "PingPong.h"
#include "Metronome.h"
#include "BlindingWhite.h"

/* KEEP IN SYNC */
unsigned char num_patterns = 4;

enum pattern_type {
  SPINNING_RAINBOW,
  PING_PONG,
  METRONOME,
  BLINDING_WHITE
};
/* KEEP IN SYNC */

// array used to display patterns
CRGB leds[NUM_LEDS];
int active_pattern_index;
Pattern* active_pattern;
int blit_interval;
int brightness;
bool paused;
long last_blit;

#define BUF_SIZE 256
// buffer for incoming commands
size_t message_length;
size_t bytes_read;
bool in_message;
char message[BUF_SIZE];

void clearLeds() {
   for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
   FastLED.show();
}

void setup() {
  brightness = 128;
  paused = true;
  active_pattern_index = 0;
  
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2400); 
  FastLED.setBrightness(brightness);
  clearLeds();

  active_pattern = new SpinningRainbow(leds);
  active_pattern->blit();
  last_blit = millis();
  blit_interval = active_pattern->updateInterval();

  Serial.begin(9600);
  Serial1.begin(9600);
  message_length = 0;
  bytes_read = 0;
  in_message = false;
}

void printMessage() {
  Serial.print("Command: ");
  message[bytes_read] = '\0';
  Serial.println(message);
}

bool checkLength(int desired_length) {
  if (desired_length != bytes_read) {
    Serial.print("Incorrect message length, expected ");
    Serial.print(desired_length);
    Serial.print(" received ");
    Serial.println(bytes_read);
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
    case 'G': {
      Serial1.write('\3');
      Serial1.write(paused ? '0' : '1');
      Serial1.write(128);
      Serial1.write(active_pattern_index);
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
      if (message[1] >= num_patterns) {
        Serial.println("Unsupported pattern received for 'S'");
        printMessage();
      } else {
        if (message[1] == active_pattern_index) break;
        if (active_pattern != nullptr) {
          delete active_pattern;
          clearLeds();
        }
        active_pattern_index = message[1];
        switch ((pattern_type) message[1]) {
          case SPINNING_RAINBOW:
            active_pattern = new SpinningRainbow(leds);
            break;
          case PING_PONG:
            active_pattern = new PingPong(leds, NUM_LEDS);
            break;
          case METRONOME:
            active_pattern = new Metronome(leds);
            break;
          case BLINDING_WHITE:
            active_pattern = new BlindingWhite(leds);
            break;
        }
        active_pattern->blit();
        last_blit = millis();
        FastLED.show();
        blit_interval = active_pattern->updateInterval();
      }
      break;
    }
  }
}

void receiveBluetooth() {
  int commands_processed = 0;
  while (Serial1.available()) {
    char read = Serial1.read();
    if (!in_message) {
      message_length = read;
      Serial.println(message_length);
      if (message_length == 0) continue;
      in_message = true;
      bytes_read = 0;
    } else {
      char received = read;
      message[bytes_read] = received;
      bytes_read++;
      if (bytes_read == message_length) {
        printMessage();
        handleCommand();
        commands_processed++;
        if (commands_processed == 5) return;
        in_message = false;
      }
    }
  }
}

void loop() 
{
  long time = millis();
  if (active_pattern != nullptr && !paused &&
      blit_interval != -1 && (time - last_blit) >= blit_interval) {
    active_pattern->blit();
    FastLED.show();
    last_blit = time;
  }
  receiveBluetooth();
  FastLED.delay(10);
}

//void scale_ping_pong() {
//  idx += pp_delta;
//  if (idx >= NUM_LEDS) {
//    pp_delta *= -1;
//    idx = NUM_LEDS - 1 + pp_delta;
//  } else if (idx < 0) {
//    pp_delta *= -1;
//    idx = pp_delta;
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
//void sin_crawl() {
//  th += 0.1;
//  double ps = 0.3;
//  double max_bright = 140;
//  for (int i = 0; i < NUM_LEDS; i++) {
//    double value = sin(th + ps * (i - (NUM_LEDS / 2)));
//    double mapped = (value * value * value * value) * max_bright;
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


