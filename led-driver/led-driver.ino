#include <SoftwareSerial.h>

#include "FastLED.h"

#define LED_DATA_PIN 12        // what output is connected to the LEDs
#define COLOR_ORDER GRB
#define NUM_LEDS 30
#define LED_TYPE WS2812B

#include "Pattern.h"
#include "SpinningRainbow.h"
#include "PingPong.h"

/* KEEP IN SYNC */
unsigned char num_patterns = 2;

enum pattern_type {
  SPINNING_RAINBOW,
  PING_PONG
};
/* KEEP IN SYNC */

// array used to display patterns
CRGB leds[NUM_LEDS];
int active_pattern_index;
Pattern* active_pattern;
bool paused;


// Swap RX/TX connections on bluetooth chip
//   Pin 10 --> Bluetooth TX
//   Pin 9 --> Bluetooth RX
SoftwareSerial btSerial(10, 9); // RX, TX

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
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  clearLeds();
  active_pattern_index = 0;
  active_pattern = new SpinningRainbow(leds, NUM_LEDS);
  paused = true;

  Serial.begin(9600);
  btSerial.begin(9600);
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
    case 'G': {
      if (message[1] == 'P') {
        btSerial.write('\1');
        btSerial.write(paused ? '0' : '1');
      }
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
            active_pattern = new SpinningRainbow(leds, NUM_LEDS);
            break;
          case PING_PONG:
            active_pattern = new PingPong(leds, NUM_LEDS);
            break;
        }
      }
      break;
    }
  }
}

void receiveBluetooth() {
  while (btSerial.available()) {
    if (!in_message) {
      message_length = btSerial.read();
      Serial.print("Incoming message, ");
      Serial.print(message_length);
      Serial.println(" bytes");
      if (message_length == 0) continue;
      in_message = true;
      bytes_read = 0;
    } else {
      char received = btSerial.read();
      message[bytes_read] = received;
      bytes_read++;
      if (bytes_read == message_length) {
        printMessage();
        handleCommand();
        in_message = false;
      }
    }
  }
}

void loop() 
{
  if (active_pattern != nullptr && !paused) {
    active_pattern->blit();
    FastLED.show();
  }
  receiveBluetooth();
  delay(50);
}


//
//void blinding_white() {
//  for (int i = 0; i < NUM_LEDS; i++) {
//    leds[i] = CRGB::White;
//  }
//  FastLED.show();
//}
//
//void dull_pulse_yellow() {
//  CRGB color = CRGB(204, 102, 0);
//  int target_bright = 30;
//  int radius = 30;
//  double delta = 0.025;
//  
//  th += delta;
//  if (th > 2 * PI) th = 0;
//  for (int i = 0; i < NUM_LEDS; i++) {
//    leds[i] = color;
//    leds[i] %= int(target_bright + abs(sin(th)) * radius);
//  }
//  FastLED.show();
//  delay(20);
//}
//
//void scale(CRGB color) {
//  for (int i = 0; i < NUM_LEDS; i++) {
//    double fraction = double(i) / NUM_LEDS;
//    leds[i] = color;
//    leds[i] %= 128 * fraction; 
//  }
//  FastLED.show();
//}
//
//void ping_pong() {
//  leds[idx] = CRGB::Black;
//  idx += pp_delta;
//  if (idx >= NUM_LEDS) {
//    pp_delta *= -1;
//    idx = NUM_LEDS - 1 + pp_delta;
//  } else if (idx < 0) {
//    pp_delta *= -1;
//    idx = pp_delta;
//  }
//  
//  leds[idx].r = 40;
//  leds[idx].b = 120;
//  FastLED.show();
//  delay(50);
//}
//
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


