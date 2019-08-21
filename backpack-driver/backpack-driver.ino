#include "FastLED.h"

#define LED_DATA_PIN 12
#define COLOR_ORDER GRB
#define NUM_LEDS 51
#define LED_TYPE WS2812B
#define RANDOM_SWITCH_INTERVAL 60000

enum pattern_type {
  SPINNING_RAINBOW,
  PING_PONG,
  SPARKLE,
  FIRE,
  PULSE,
  METEOR,
  MOVING_MOUND,
  THEATER_CHASE,
  PATTERN_COUNT, // keep as second-to-last
};

// array used to display patterns
CRGB leds[NUM_LEDS];
long lastBlit;

// pattern local vars
int work_a[NUM_LEDS];
int work_b[NUM_LEDS];
byte work_c[NUM_LEDS];

int index;
int radius;
double theta;

#define NUM_BALLS 5
int *indices = work_a;
int *deltas = work_b;
CRGB colors[NUM_BALLS];

int *ages = work_a;
int *lifetimes = work_b;

byte *heat = work_c;

int delta;

#define NUM_MOUNDS 3
bool bounces[NUM_MOUNDS];
byte* moundArrays[NUM_MOUNDS] { (byte *) work_a, (byte *)work_b, work_c };
double float_indices[NUM_MOUNDS];
double float_deltas[NUM_MOUNDS];


// current pattern settings
pattern_type activePattern;
byte brightness = 190;
long lastRandomSwitch;

void setup() {
  Serial.begin(9600);
  Serial.println("");
  Serial.println("LED Driver is starting");

  randomSeed(analogRead(0));
  lastRandomSwitch = millis();

  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2400);
  FastLED.setBrightness(brightness);
  clearLeds();

  setPattern(SPINNING_RAINBOW);
  Serial.println("LED Driver has finished setup");
}

void loop() {
  long t = millis();

  if (t - lastRandomSwitch > RANDOM_SWITCH_INTERVAL) {
    switchToRandomPattern();
    lastRandomSwitch = t;
  }

  int interval = getUpdateInterval(); 
  if (interval != -1 && (t - lastBlit) >= interval) {
    displayPattern();
    FastLED.show();
    lastBlit = t;
  }

  FastLED.delay(10);
}

void switchToRandomPattern() {
    pattern_type next = activePattern;
    while (next == activePattern) next = (pattern_type) random(0, PATTERN_COUNT);
    clearLeds();
    setPattern(next);
}

void setPattern(pattern_type p) {
  activePattern = p;
   switch (p) {
    case SPINNING_RAINBOW:
      radius = 75;
      index = 0;
      theta = 0.0;
      break;
    case PING_PONG:
      for (int i = 0; i < NUM_BALLS; i++) {
        colors[i] = CHSV(random(255), 255, 255);
        indices[i] = random(0, NUM_LEDS);
        deltas[i] = random(2) * 2 - 1;
      }
      break;
    case SPARKLE:
      for (int i = 0; i < NUM_LEDS; i++) {
        lifetimes[i] = random(1000, 6000);
        bool alive = random(0, 4) == 0;
        if (alive) {
          ages[i] = 0;
        } else {
          ages[i] = random(-10000, -3000);
        }
        leds[i] = calculateSparkleColor(i);
      }
    case FIRE:
      for (int i = 0; i < NUM_LEDS; i++) heat[i] = 0;
      break;
    case PULSE:
      theta = 0;
      colors[0] = CHSV(random(255), 255, 255);
      break;
    case METEOR:
      index = 0;
      radius = 3;
      delta = 1;
      colors[0] = CHSV(random(255), 255, 255);
      break;
    case MOVING_MOUND:
      radius = 9;
      for (int i = 0; i < NUM_MOUNDS; i++) {
        float_indices[i] = 0.0;
        bounces[i] = false;
        moundArrays[i][radius] = 1;
        for (int j = 1; j <= radius; j++) {
          moundArrays[i][radius + j] = min(pow(2, j), 255);
          moundArrays[i][radius - j] = min(pow(2, j), 255);
        }
      }
      bounces[1] = true;
      colors[0] = CRGB::Red;
      colors[1] = CRGB::Green;
      colors[2] = CRGB::Blue;
      float_deltas[0] = 1;
      float_deltas[1] = .8;
      float_deltas[2] = .6;
      break;
    case THEATER_CHASE:
      colors[0] = CHSV(random(255), 255, 255);
      index = 0;
      break;
   }
}

void displayPattern() {
  switch (activePattern) {
    case SPINNING_RAINBOW: {
      const double delta = 0.1;
      const double valueTarget = 80.0;
      const double valueRadius = 15.0;
      index--;
      index %= radius;

      theta += delta;
      if (theta > 2 * PI) theta = 0.0;
      for (int i = 0; i < NUM_LEDS; i++) {
        double hue = double((i + index) % radius) / radius * 255;
        double value = valueTarget + sin(theta) * valueRadius;
        leds[i] = CHSV(int(hue), 255, int(value));
      }
      break;
    }
    case PING_PONG:
      for (int i = 0; i < NUM_BALLS; i++) {
        leds[indices[i]] = CRGB::Black;
        indices[i] += deltas[i];
        if (indices[i] >= NUM_LEDS) {
          deltas[i] = -1;
          indices[i] = NUM_LEDS - 2;
        } else if (indices[i] < 0) {
          deltas[i] = 1;
          indices[i] = 1;
        }
  
        leds[indices[i]] = colors[i];
      }
      break;
    case SPARKLE: {
      int delta = millis() - lastBlit;
      for (int i = 0; i < NUM_LEDS; i++) {
        ages[i] += delta;
        if (ages[i] > lifetimes[i]) {
          lifetimes[i] = random(1000, 6000);
          ages[i] = random(-10000, -3000);
        }
        leds[i] = calculateSparkleColor(i);
      }
      break;
    }
    case FIRE: {
      const int cooling = 15;
      const int sparking = 55;
      for (int i = 0; i < NUM_LEDS; i++) {
        int cooldown = random(0, ((cooling * 10) / 12) + 2);
       
        if (cooldown > heat[i]) {
          heat[i] = 0;
        } else {
          heat[i] = heat[i] - cooldown;
        }
      }
      
      // Step 2.  Heat from each cell drifts 'up' and diffuses a little
      for (int k = NUM_LEDS - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + 2 * heat[k - 2]) / 3;
      }
      
      // Step 3.  Randomly ignite new 'sparks' near the bottom
      if (random(255) < sparking) {
        int index = random(3);
        heat[index] = heat[index] + random(160,255);
      }

      // Step 4.  Convert heat to LED colors
      for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = calculateFireColor(heat[j]);
      }
      break;
    }
    case PULSE:
      theta += 0.1;
      if (theta > 2 * PI) theta -= 2 * PI;
      
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = colors[0];
        double modifier = 180 * max((sin(theta) + .2)/1.2, 0.1);
        leds[i] %= int(modifier);
      }
      break;
    case METEOR: {
      // fade brightness all LEDs one step
      for (int j = 0; j < NUM_LEDS; j++) {
        if (random(10) > 3) {
          leds[j].fadeToBlackBy(90);
        }
      }
    
      // draw meteor
      for (int j = 0; j < radius; j++) {
        if (0 <= index - j && index - j < NUM_LEDS) {
          leds[index - j] += colors[0];
        }
      }
      index += delta;
      if (index == -1 || index == NUM_LEDS) {
        delta *= -1;
        index += 2 * delta;
        colors[0] = CHSV(random(255), 255, 255); 
      }
      break;
    }
    case MOVING_MOUND: {
      // Erase old LEDs
      for (int i = 0; i < NUM_MOUNDS; i++) {
        index = round(float_indices[i]);
        leds[index] -= colors[i] / moundArrays[i][radius];
        for (int k = 1; k <= radius; k++) {
          // Clear trailing LEDs
          int led_index = index - k;
          if (led_index > 0) {
            leds[led_index] -= colors[i] / moundArrays[i][radius - k];
          } else if (!bounces[i]) {
            leds[led_index + NUM_LEDS - 1] -= colors[i] / moundArrays[i][radius - k];
          }
      
          // Clear leading LEDs
          led_index = index + k;
          if (led_index < NUM_LEDS) {
            leds[led_index] -= colors[i] / moundArrays[i][radius - k];
          } else if (!bounces[i]) {
            leds[led_index - NUM_LEDS] -= colors[i] / moundArrays[i][radius - k];
          }
        }
      
        float_indices[i] += float_deltas[i]; // Move center of mound
        index = round(float_indices[i]);
      
        // Check bounce/loop conditions
        if (index >= NUM_LEDS) {
          if (bounces[i]) {
            float_deltas[i] *= -1;
            float_indices[i] = NUM_LEDS - 2;
          } else {
            float_indices[i] = 0;
          }
        } else if (index < 0) {
          float_deltas[i] *= -1;
          float_indices[i] = 1;
        }
        
        index = round(float_indices[i]);
      
        // Set main LED
        leds[index] = colors[i];
        // Draw the trailing and leading LEDs
        for (int k = 1; k <= radius; k++) {
          // Draw trailing LEDs
          int led_index = index - k;
          if (led_index > 0) {
            leds[led_index] += colors[i] / moundArrays[i][radius - k];
          } else if (bounces[i]) {
            leds[led_index + NUM_LEDS - 1] += colors[i] / moundArrays[i][radius - k];
          }
      
          // Draw leading LEDs
          led_index = index + k;
          if (led_index < NUM_LEDS) {
            leds[led_index] += colors[i] / moundArrays[i][radius - k];
          } else if (bounces[i]) {
            leds[led_index - NUM_LEDS] += colors[i] / moundArrays[i][radius - k];
          }
        }
      }
    break;
    }
    case THEATER_CHASE: {
      index += 1;
      index %= 3;
    
      for (int i = 0; i < NUM_LEDS; i++) {
        if (i % 3 == index) {
          leds[i] = colors[0]; 
        } else {
          leds[i] = CRGB::Black;
        }
      }

      break;
    }
  }
}

CRGB calculateSparkleColor(int index) {
  double lifeFraction = double(ages[index]) / lifetimes[index];
  if (ages[index] < 0 || lifeFraction >= 1) return CRGB::Black;
  if (lifeFraction < 0.8) {
    double intervalFraction = lifeFraction / 0.8;
    return CHSV(28 * intervalFraction, 255, intervalFraction * 70 + 185);
  }
  if (lifeFraction > 0.8) {
    double intervalFraction = (lifeFraction - 0.8) / 0.2;
    return CHSV(28 * (1 - intervalFraction), 255, (1 - intervalFraction) * 255);
  }
}

CRGB calculateFireColor(byte temperature) {
  // figure out which third of the spectrum we're in:
  int lowStart = 0;
  int midStart = 100;
  int highStart = 200;
  
  if (temperature > highStart) {
    // hottest
    float heatramp = ((float) (temperature - highStart)) / (255 - highStart);
    return CHSV(35, 255 - (heatramp * 230) , 255);
  } else if (temperature > midStart) {
    float heatramp = ((float) (temperature - midStart)) / (highStart - midStart);
    // middle
    return CHSV(heatramp * 25 + 10, 255, 255);
  } else {
    float heatramp = ((float) (temperature - lowStart)) / (midStart - lowStart);
    // coolest
    return CHSV(10, 255, heatramp * 255);
  }
}


bool usesUserColor(pattern_type p) {
  switch (p) {
    case SPARKLE:
    case PULSE:
      return true;
    default:
      return false;
  }
}

long getUpdateInterval() {
  switch(activePattern) {
    case FIRE: return 55;
    case METEOR: return 60;
    case SPINNING_RAINBOW: return 80;
    case THEATER_CHASE: return 150;
    default: return 50;
  }
}

void clearLeds() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

