#pragma once

#include "./common.h"

double lateralChances[8] = {0.9, 0.83, 0.72, 0.62, 0.4, -1, -1};

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