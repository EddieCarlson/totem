#pragma once

#include <FastLED.h>
#include <Bounce2.h>

const int NUM_ROWS = 75;
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

CRGB strip0[NUM_ROWS];
CRGB strip1[NUM_ROWS];
CRGB strip2[NUM_ROWS];
CRGB strip3[NUM_ROWS];
CRGB strip4[NUM_ROWS];
CRGB strip5[NUM_ROWS];
CRGB strip6[NUM_ROWS];
CRGB strip7[NUM_ROWS];

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
  return (row + NUM_ROWS) % NUM_ROWS;
}

int topRowIdxForCol(int col) {
    if (col == MISSING_PIXEL_STRIP) {
        return NUM_ROWS - 2;
    } else {
        return NUM_ROWS - 1;
    }
}

// pixel 0 is near the top - this lets us have pixel/row 0 at bot of column
int upRow(int row, int col) {
    return topRowIdxForCol(col) - row;
}

void fadePixel(int col, int _row, double factor) {
  int row = upRow(_row, col);
  if (col >= 0 && col < 8 && row >=0 && row < NUM_ROWS) {
    strip(col)[row] = strip(col)[row].nscale8(factor * 256);
  }
}

void setPixel(int col, int _row, CRGB color) {
  int row = upRow(_row, col);
  if (col >= 0 && col < 8 && row >= 0 && row < NUM_ROWS) {
    strip(col)[row] = color;
  }
}

void setPixel(int col, int _row, CRGB color, double fade) {
  int row = upRow(_row, col);
  if (col >= 0 && col < 8 && row >= 0 && row < NUM_ROWS) {
    strip(col)[row] = color;
  }
  fadePixel(col, row, fade);
}

CRGB getPixel(int col, int _row) {
  int row = upRow(_row, col);
  if (col >= 0 && col < 8 && row >= 0 && row < NUM_ROWS) {
    return strip(col)[row];
  } else {
    return CRGB::Black;
  }
}

void fadeAll(double factor) {
  for (int i = 0; i < NUM_ROWS; i++) {
    for (int s = 0; s < 8; s++) {
      fadePixel(s, i, factor);
    }
  }
}


void stripNumbering() {
  for (int i = 0; i < 8; i++) {
    for (int r = 0; r <= i; r++) {
      setPixel(i, r, CRGB::Blue);
      setPixel(i, NUM_ROWS - 1 - r, CRGB::Blue);
    }
  }
}

void setAll(CRGB color) {
  for (int c = 0; c < 8; c++) {
    for (int r = 0; r < NUM_ROWS; r++) {
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
