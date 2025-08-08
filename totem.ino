#include <arduino.h>
#include <math.h>

#include <FastLED.h>
#include <Bounce2.h>

#include "./common.h"
#include "./timeOval.h"
#include "./walker.h"

int iterationCount = 0;



Walker* w;
Walker* w2;
Walker* w3;
TimeOval* t;

double colFun(uint32_t t) {
  double seconds = (double) t / 1000.0;
  double val = 0.5 + 1.25 * ((seconds * 4) + (8 * ((1.0 + (cos(seconds * 2 * PI / 20))) / 2.0)));

  return fmod(val, 8.0);
}

double rowFun(uint32_t t) {
  double seconds = (double) t / 1000.0;
  
  return (seconds * 15) + (5 * cos(seconds * 2 * PI / 13.0));
}

double slopeFun(uint32_t t) {
  double seconds = (double) t / 1000.0;
  return (20 * ((1.1 + cos(seconds * 2.0 * PI / 23.0)) / 2.0));
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

// double lateralChances[4] = {0.7, 0.6, 0.3, 0};
int curWalkPixel[2] = {0, 0};
int curWalkDir = 0;
int numUps = 1;
int prevLaterals = 0;
double walkFade = 0.91;

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
