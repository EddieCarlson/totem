#pragma once

#include <FastLED.h>
#include <Bounce2.h>

const int NUM_LEDS = 75;
const int PIN_0 = 2;
const int PIN_1 = 3;
const int PIN_2 = 7;
const int PIN_3 = 4;
const int PIN_4 = 8;
const int PIN_5 = 9;
const int PIN_6 = 15;
const int PIN_7 = 14;

const int MISSING_PIXEL_STRIP = 6;

const int BUTTON1_PIN = 11;
const int BUTTON2_PIN = 12;

CRGB strip0[NUM_LEDS];
CRGB strip1[NUM_LEDS];
CRGB strip2[NUM_LEDS];
CRGB strip3[NUM_LEDS];
CRGB strip4[NUM_LEDS];
CRGB strip5[NUM_LEDS];
CRGB strip6[NUM_LEDS];
CRGB strip7[NUM_LEDS];

const int RAINBOW_HUES = 256;
CRGB rainbow[RAINBOW_HUES];

Bounce2::Button button1 = Bounce2::Button();
Bounce2::Button button2 = Bounce2::Button();

typedef double (*TimeFunc)(uint32_t);

CRGB* strip(int index) { 
  index = (index + 8) % 8;
  if (index == 0) {
    return strip0;
  } else if (index == 1) {
    return strip1;
  } else if (index == 2) {
    return strip2;
  } else if (index == 3) {
    return strip3;
  } else if (index == 4) {
    return strip4;
  } else if (index == 5) {
    return strip5;
  } else if (index == 6) {
    return strip6;
  } else if (index == 7) {
    return strip7;
  } else {
    return strip7;
  }
}

double rand01() {
  return (double) rand() / (double) RAND_MAX;
}

int safeRow(int row) {
  return (row + NUM_LEDS) % NUM_LEDS;
}

// pixel 0 is near the top - this lets us have pixel/row 0 at bot of column
int upRow(int row, int col) {
  if (col == MISSING_PIXEL_STRIP) {
    return NUM_LEDS - row - 2;
  }
  return NUM_LEDS - row - 1;
}

void setPixel(int col, int _row, CRGB color) {
  int row = upRow(_row, col);
  if (col >= 0 && col < 8 && row >= 0 && row < NUM_LEDS) {
    strip(col)[row] = color;
  }
}

CRGB getPixel(int col, int _row) {
  int row = upRow(_row, col);
  if (col >= 0 && col < 8 && row >= 0 && row < NUM_LEDS) {
    return strip(col)[row];
  } else {
    return CRGB::Black;
  }
}

void fadePixel(int col, int _row, double factor) {
  int row = upRow(_row, col);
  if (col >= 0 && col < 8 && row >=0 && row < NUM_LEDS) {
    strip(col)[row] = strip(col)[row].nscale8(factor * 256);
  }
}

void fadeAll(double factor) {
  for (int i = 0; i < NUM_LEDS; i++) {
    for (int s = 0; s < 8; s++) {
      fadePixel(s, i, factor);
    }
  }
}


void stripNumbering() {
  for (int i = 0; i < 8; i++) {
    for (int r = 0; r <= i; r++) {
      setPixel(i, r, CRGB::Blue);
      setPixel(i, NUM_LEDS - 1 - r, CRGB::Blue);
    }
  }
}

void setAll(CRGB color) {
  for (int c = 0; c < 8; c++) {
    for (int r = 0; r < NUM_LEDS; r++) {
      setPixel(c, r, color);
    }
  }
}

void clearAll() {
  setAll(CRGB::Black);
}

class TimeFuncS {
  public:
    typedef double (*TF)(uint32_t);

    TF tf;
    bool initialized;
    uint32_t initMillis;

    TimeFuncS(TF f) {
      initialized = false;
      tf = f;
    }

    double update() {
      uint32_t t = millis();
      if (!initialized) {
        initialized = true;
        initMillis = t;
      }
      return tf(t - initMillis);
    }
};
