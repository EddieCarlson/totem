#include <arduino.h>
#include <FastLED.h>
#include <math.h>

#include <Bounce2.h>

#define NUM_LEDS 75
#define PIN_0 2
#define PIN_1 3
#define PIN_2 7
#define PIN_3 4
#define PIN_4 8
#define PIN_5 9
#define PIN_6 15
#define PIN_7 14

#define MISSING_PIXEL_STRIP 6

#define BUTTON1_PIN 11
#define BUTTON2_PIN 12

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

double lateralChances[8] = {0.9, 0.83, 0.72, 0.62, 0.4, -1, -1};

int iterationCount = 0;

// class PeriodFunc {
//   public:
//     double maxVal;
//     double minVal;
//     double periodMillis;
//     double startVal;
//     double curVal;
//     bool startUp;
//     bool curUp;
//     int mode; // 0 = linear, 1 = sine
//     int integralMillis;
//     uint32_t lastTimestamp;

//     TimeFunc(double _maxVal, double _minVal, double _startVal, double _curVal, bool _startUp, bool _curUp, int _mode) {
      
//     }


//     double periodFraction() {
//       return ((double) (millis() - lastTimestamp)) / periodMillis;
//     }

//     double update() {
//       double frac = fmod(periodFraction(), periodMillis);

//     }




// }

const double root2 = sqrt(2) / 2.0;
const double negRoot2 = -1 * root2;

double colCoords[8][2] = {{1.0, 0.0}, {root2, root2}, {0.0, 1.0}, {negRoot2, root2}, {-1.0, 0.0}, {negRoot2, negRoot2}, {0.0, -1.0}, {root2, negRoot2}};

class TimeOval {
  public:
    typedef double (*TimeFunc)(uint32_t);
    bool initialized;
    uint32_t initializedMillis;
    TimeFunc baseRowAtTime;
    TimeFunc baseColAtTime;
    TimeFunc slopeAtTime;
    double curBaseRow;
    double curBaseCol;
    double curSlope;

    TimeOval(TimeFunc _row, TimeFunc _col, TimeFunc _slope) {
      baseRowAtTime = _row;
      baseColAtTime = _col;
      slopeAtTime = _slope;
      initialized = false;
      initializedMillis = 0;
    }

    void update() {
      uint32_t t = millis();
      if (!initialized) {
        initializedMillis = t;
        initialized = true;
      }
      curBaseRow = baseRowAtTime(t - initializedMillis);
      curBaseCol = baseColAtTime(t - initializedMillis);
      curSlope = slopeAtTime(t - initializedMillis);
    }

    double zAtCol(int col) {
      double curX = cos(curBaseCol * PI / 4.0);
      double curY = sin(curBaseCol * PI / 4.0);

      double colX = colCoords[col][0];
      double colY = colCoords[col][1];

      return (-1 * (((curX * colX) - (curX * curX)) + ((curY * colY) - (curY * curY))) * curSlope) + curBaseRow;
    }
};



class Walker {
  public:
    int numUps;
    int curCol;
    int curRow;
    int prevLaterals;
    int curWalkDir; // -1 = left, 0 = up, 1 = right
    CRGB color;

    Walker(int _numUps, int _curCol, int _curRow, int _prevLaterals, int _curWalkDir, CRGB _color) {
      numUps =_numUps;
      curCol = _curCol;
      curRow = _curRow;
      prevLaterals = _prevLaterals;
      curWalkDir = _curWalkDir;
      color = _color;
    }

    void advance() {
      double randVal = (double) rand() / (double) RAND_MAX;
      if (numUps > 1 && prevLaterals < 8 && randVal < lateralChances[prevLaterals]) {
        if (curWalkDir == 0) {
            double dirChoice = (double) rand() / (double) RAND_MAX;
            if (dirChoice > 0.5) {
              curWalkDir = -1;
            } else {
              curWalkDir = 1;
            }
        }
        numUps = 0;
        prevLaterals = prevLaterals + 1;
        curCol = ((curCol + curWalkDir) + 8) % 8;
      } else {
        numUps = numUps + 1;  
        curWalkDir = 0;
        prevLaterals = 0;
        curRow = (curRow + 1) % NUM_LEDS;
      }
    }
};

Walker* w;
Walker* w2;
Walker* w3;
TimeOval* t;

double colFun(uint32_t t) {
  double seconds = (double) t / 1000.0;
  double val = 1.25 * ((seconds * 4) + (8 * ((1.0 + (cos(seconds * 2 * PI / 20))) / 2.0)));

  return fmod(val, 8.0);
}

double rowFun(uint32_t t) {
  double seconds = (double) t / 1000.0;
  
  return (seconds * 10) + (10 * cos(seconds * 2 * PI / 10.0));
}

double slopeFun(uint32_t t) {
  double seconds = (double) t / 1000.0;
  return 3 + (15 * ((0.9 + cos(seconds * 2 * PI / 13.0)) / 2.0));
}

double zero(uint32_t t) {
  return 0.0;
}

void setup() {
  FastLED.addLeds<WS2812B, PIN_0, RGB>(strip0, NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_1, RGB>(strip1, NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_2, RGB>(strip2, NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_3, RGB>(strip3, NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_4, RGB>(strip4, NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_5, RGB>(strip5, NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_6, RGB>(strip6, NUM_LEDS);
  FastLED.addLeds<WS2812B, PIN_7, RGB>(strip7, NUM_LEDS);

  fill_rainbow(rainbow, RAINBOW_HUES, 0, 1);

  button1.attach(11, INPUT_PULLUP);
  button1.interval(12);
  button1.setPressedState(LOW);

  button2.attach(12, INPUT_PULLUP);
  button2.interval(12);
  button2.setPressedState(LOW);

  w = new Walker(0, 0, 0, 0, 0, CRGB::Cyan);
  w2 = new Walker(0, 0, 0, 0, 0, CRGB::Gold);
  w3 = new Walker(0, 0, 0, 0, 0, CRGB::Purple);
  t = new TimeOval(rowFun, colFun, slopeFun);
}

int curCol = 0;

void buttonCheck() {
  button1.update();
  button2.update();

  if (button1.pressed()) {
    curCol = (curCol + 7) % 8;
  }
  if (button2.pressed()) {
    curCol = (curCol + 1) % 8;
  }
}

void buttonShow() {
  buttonCheck();

  for (int c = 0; c < 8; c++) {
    for (int r = 0; r < NUM_LEDS; r++) {
      setPixel(c, r, CRGB::Black);
    }
  }

  int nextCol = (curCol + 1) % 8;
  for (int i = 0; i < NUM_LEDS; i++) {
    setPixel(curCol, i, CRGB::Green);
    setPixel(nextCol, i, CRGB::Blue);
  }
  fadeAll(0.2);
}

int curHue = 0;

void advanceHue() {
  curHue = (curHue + 1) % RAINBOW_HUES;
}

double cSpeed = 20;
double rSpeed = 3;
bool cSpeedUp = true;
bool rSpeedUp = true;
int cSpeedSameDirTimes = 0;
int rSpeedSameDirTimes = 0;
double cSpeedIncrement = 0.01;
double rSpeedIncrement = 0.002;

void setSpeeds() {
  double cSpeedSameDirChance = pow(0.995, cSpeedSameDirTimes);
  if (rand01() > cSpeedSameDirChance) {
    cSpeedUp = !cSpeedUp;
  }
  double rSpeedSameDirChance = pow(0.995, rSpeedSameDirTimes);
  if (rand01() > rSpeedSameDirChance) {
    rSpeedUp = !rSpeedUp;
  }
  if (cSpeedUp) {
    cSpeed += cSpeedIncrement;
  } else {
    cSpeed -= cSpeedIncrement;
  }
  if (rSpeedUp) {
    rSpeed += rSpeedIncrement;
  } else {
    rSpeed -= rSpeedIncrement;
  }
}

double cStartFactor = 17.0;
double rStartFactor = 1.2;
double cFactor = cStartFactor;
double rFactor = rStartFactor;
int speedUpCTimesMax = 800;
int curCTimes = (speedUpCTimesMax * 0.8);
bool cUp = true;
int speedUpRTimesMax = 519;
int curRTimes = 0;
bool rUp = true;


void setSpeeds2() {
  if (curCTimes > speedUpCTimesMax) {
    cUp = !cUp;
    curCTimes = 0;
  } else {
    curCTimes++;
  }
  if (curRTimes > speedUpRTimesMax) {
    rUp = !rUp;
    curRTimes = 0;
  } else {
    curRTimes++;
  }
  double baseCMod = (cStartFactor / ((double) speedUpCTimesMax)) * 2.0;
  double cMod = baseCMod + ((1.0 - rand01()) * baseCMod * 0.04);
  if (cUp) {
    cFactor += cMod;
  } else {
    cFactor -= cMod;
  }
  double baseRMod = (rStartFactor / ((double) speedUpRTimesMax)) * 0.65;
  double rMod = baseRMod + ((1.0 - rand01()) * baseRMod * 0.04);
  if (rUp) {
    rFactor += rMod;
  } else {
    rFactor -= rMod;
  }
}

void advanceRainbow() {
  setSpeeds2();
  for (int c = 0; c < 8; c++) {
    for (int r = 0; r < NUM_LEDS; r++) {
      int colorIndex = ((int) (((double) curHue) + (cFactor * c) + (rFactor * r)) + RAINBOW_HUES) % RAINBOW_HUES;
      // int colorIndex = max(min(round(fmod(((double) curHue) + (cSpeed * c) + (rSpeed * r), RAINBOW_HUES)), RAINBOW_HUES), 0);
      // int colorIndex = (curHue + (r) + (c * 6)) % RAINBOW_HUES; 
      setPixel(c, r, rainbow[colorIndex]);
    }
  }
  advanceHue();
}

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

// double lateralChances[4] = {0.7, 0.6, 0.3, 0};
int curWalkPixel[2] = {0, 0};
int curWalkDir = 0;
int numUps = 1;
int prevLaterals = 0;
double walkFade = 0.91;

double rand01() {
  return (double) rand() / (double) RAND_MAX;
}

int safeRow(int row) {
  return (row + NUM_LEDS) % NUM_LEDS;
}

void setPixelAA(double col, double row, CRGB color) {
  for (int c = 0; c < 8; c++) {
    for (int r = row - 4; r <= row + 4; r++) {
      double distSq = pow(abs(r - row), 2) + pow(abs(c - col), 2);
      double factor = (1.0 / pow((distSq + 1.5), 3.7)) / 0.08;
      if (factor < 0.05) {
        factor = 0;
      }
      int safeR = safeRow(r);
      setPixel(c, safeR, color);
      fadePixel(c, safeR, factor);
    }
  }
}

void setNextWalkP(Walker* z) {
  fadePixel(z->curCol, z->curRow, 0.5);
  z->advance();
  // setPixel(curWalkPixel[0], curWalkPixel[1], color);
  setPixel(z->curCol, NUM_LEDS - z->curRow, z->color);
}

void setNextWalkPixel3() {
  setNextWalkP(w);
  if (iterationCount > 24) {
    setNextWalkP(w2);
  }
  if (iterationCount > 49) {
    setNextWalkP(w3);
  }
  fadeAll(walkFade);
}

void setNextWalkPixel(CRGB color) {
  fadePixel(curWalkPixel[0], curWalkPixel[1], 0.5);
  double randVal = (double) rand() / (double) RAND_MAX;
  if (numUps > 1 && prevLaterals < 8 && randVal < lateralChances[prevLaterals]) {
    if (curWalkDir == 0) {
        double dirChoice = (double) rand() / (double) RAND_MAX;
        if (dirChoice > 0.5) {
          curWalkDir = -1;
        } else {
          curWalkDir = 1;
        }
    }
    numUps = 0;
    prevLaterals = prevLaterals + 1;
    curWalkPixel[0] = ((curWalkPixel[0] + curWalkDir) + 8) % 8;
  } else {
    numUps = numUps + 1;  
    curWalkDir = 0;
    prevLaterals = 0;
    curWalkPixel[1] = (curWalkPixel[1] + 1) % NUM_LEDS;
  }
  fadeAll(walkFade);
  //setPixel(curWalkPixel[0], curWalkPixel[1], color);
  setPixel(curWalkPixel[0], NUM_LEDS - curWalkPixel[1], color);
}

double firstLateralChances2[10] = {0.0, 0.02, 0.05, 0.1, 0.2, 0.2, 0.2, 0.25, 0.3, 0.4};
double lateralChance2Start = 0.7;
double curLateralChance = lateralChance2Start;
int countSinceLastUp = 0;
double curWalkPixel2[2] = {0.0, 0.0};
int curWalkDir2 = 0;

double lateralChance2() {
  if (countSinceLastUp < 10) {
    return firstLateralChances2[countSinceLastUp];
  } else {
    curLateralChance = curLateralChance * 0.987;
    countSinceLastUp++;
    return curLateralChance;
  }
}

void setNextWalkPixel2(CRGB color) {
  if (rand01() < lateralChance2()) {
    if (curWalkDir2 == 0) {
        double dirChoice = (double) rand() / (double) RAND_MAX;
        if (dirChoice > 0.5) {
          curWalkDir2 = -1;
        } else {
          curWalkDir2 = 1;
        }
    }
    countSinceLastUp = 0;
    curWalkPixel2[0] = fmod((curWalkPixel2[0] + (curWalkDir2 * 0.1) + 4), 4);
  } else {
    countSinceLastUp++;  
    curWalkDir2 = 0;
    countSinceLastUp = 0;
    curWalkPixel2[1] = fmod((curWalkPixel2[1] + 0.1), NUM_LEDS);
  }
  fadeAll(walkFade);
  setPixelAA(curWalkPixel2[0], curWalkPixel2[1], color);
}

void stripNumbering() {
  for (int i = 0; i < 8; i++) {
    for (int r = 0; r <= i; r++) {
      setPixel(i, r, CRGB::Blue);
      setPixel(i, NUM_LEDS - 1 - r, CRGB::Blue);
    }
  }
}

void clearAll() {
  setAll(CRGB::Black);
}

void setAll(CRGB color) {
  for (int c = 0; c < 8; c++) {
    for (int r = 0; r < NUM_LEDS; r++) {
      setPixel(c, r, color);
    }
  }
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

double colFadeF(uint32_t t) {
  double seconds = ((double) t) / 1000.0;

  return 64.0 + 196.0 * ((1.0 + sin(seconds * 2 * PI / 120.0)) / 2.0);
}

TimeFuncS* colFadeRange = new TimeFuncS(colFadeF);

void rainbowOval() {
  double rhd = (double) RAINBOW_HUES;
  t->update();
  double cfr = colFadeRange->update();
  for (int c = 0; c < 8; c++) {
    double z = t->zAtCol(c);
    for (int r = 0; r < NUM_LEDS; r++) {
      double dr = (double) r;
      double dist = z + ((double) NUM_LEDS) - dr;
      int colorIndexF = (int) (cfr * dist / ((double) NUM_LEDS));
      int colorIndex = ((colorIndexF % RAINBOW_HUES) + RAINBOW_HUES) % RAINBOW_HUES;
      if (colorIndex > 256 || colorIndex < 0) {
        Serial.println(colorIndex);
      }
      // int colorIndex = max(min(round(fmod(((double) curHue) + (cSpeed * c) + (rSpeed * r), RAINBOW_HUES)), RAINBOW_HUES), 0);
      // int colorIndex = (curHue + (r) + (c * 6)) % RAINBOW_HUES; 
      setPixel(c, r, rainbow[colorIndex]);
    }
  }
}



void loop() {

  rainbowOval();
  fadeAll(0.14);
  // setAll(rainbow[(iterationCount * 10) % RAINBOW_HUES]);
  // fadeAll(0.12);
  // FastLED.show();
  // delay(1000);

  // for (int c = 0; c < 8; c++) {
  //   for (int r = 10; r < NUM_LEDS - 10; r++) {
  //     setPixel(c, r, CRGB::Cyan);
  //   }
  // }
  // fadeAll(0.2);


  // setNextWalkPixel3();
  // delay(6);
  // buttonShow();
  // advanceRainbow();
  // fadeAll(0.17);
  // stripNumbering();

  // for (int c = 0; c < 8; c++) {
  //   setPixel(c, NUM_LEDS - 1, CRGB::Blue);
  // }

  // clearAll();
  // t->update();
  // for (int c = 0; c < 8; c++) {
  //   double dz = t->zAtCol(c);
  //   Serial.print(c);
  //   Serial.print(": ");
  //   Serial.println(dz);
  //   int z = round(dz);
  //   double fades[5] = {0.1, 0.3, 0.99, 0.3, 0.1};
  //   int count = -1;
  //   for (int r = z - 4; r <= z+4; r++) {
  //     count++;
  //     double fadeFactor = (double) (1.0 / (1.01 + powf(5, abs(dz - ((double) r)))));
  //     setPixel(c, r, CRGB::Purple);
  //     fadePixel(c, r, fadeFactor);
  //   }
  // }
  


  // for(int r = 0; r < 30; r++) {
  //   setPixel(0, r, CRGB::Red);
  //   setPixel(1, r, CRGB::Green);
  //   setPixel(2, r, CRGB::Blue);
  //   setPixel(3, r, CRGB::Purple);
  //   setPixel(4, r, CRGB::Aqua);
  //   setPixel(5, r, CRGB::White);
  //   setPixel(6, r, CRGB::Crimson);
  //   setPixel(7, r, CRGB::Gold);
  // }
  // fadeAll(0.2);
  

  // setPixel(0, 10, CRGB::Blue);
  // setPixel(1, 10, CRGB::Red);
  // setPixel(2, 10, CRGB::Green);
  // setPixel(3, 10, CRGB::Purple);
  // setPixel(4, 10, CRGB::Blue);
  // setPixel(5, 10, CRGB::Red);
  // setPixel(6, 10, CRGB::Green);
  // setPixel(7, 10, CRGB::Purple);

  FastLED.show();
  delay(45);
  iterationCount++;
}
