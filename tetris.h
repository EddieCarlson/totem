#pragma once

#include "./common.h"

class Tetris {
  public:
    typedef double (*TimeFunc)(uint32_t);

    double inFlightP[8][2];
    bool inFlight[8][2];
    int stack[8];
    uint32_t initMillis;
    int deployIntervalMillis;
    TimeFunc velocity;
    int matches[100][2];

    int totalInFlight() {
      int count = 0;
      for (int i = 0; i < 8; i++) {
        for (int p = 0; p < 2; p++) {
          if (inFlight[i][p]) {
            count++;
          }
        }
      }
      return count;
    }

    int inFlightByCol(int col) {
      int count = 0;
      if (inFlight[col][0]) { count++; }
      if (inFlight[col][1]) { count++; }
      return count;
    }

    int newCol() {
      int c = (int) ((rand01() * 8) + 1);
      while (inFlightByCol(c) == 2) {
        c = (int) ((rand01() * 8) + 1);
      }
      return c;
    }

    void launchNewPixel() {
      int col = newCol();
      int p = 0;
      if (inFlight[col][p]) {
        p = 1;
      }
      inFlight[col][p] = true;
      inFlightP[col][p] = 0.0;
    }

    void resetMatches() {
      for (int i = 0; i < 100; i++) {
        for (int r = 0; r < 2; r++) {
          matches[i][r] = -1;
        }
      }
    }

    // void checkCollision(int col, int row, CRGB color, int count) {
    //   int theCount = count;
    //   matches[theCount] = {col, row};
    //   theCount++;
    //   if (getPixel(col, row + 1) == color) {
    //     theCount = checkCollision(col, row + 1, color, theCount);
    //   }
    //   if (stack[col - 1] >= row && getPixel(col - 1, row) == color) {
    //     theCount++;
    //     theCount += checkCollision(col - 1, row, color, 0);
    //   }
    //   if (stack[col - 1] >= row && getPixel(col - 1, row) == color) {
    //     theCount++;
    //     theCount += checkCollision(col - 1, row, color, 0);
    //   }
    // }

    // void checkCollision(int col) {
    //   resetMatches();
    //   checkCollision(col, stack[col], getPixel(col, row), 0);
    // }

    void advance() {
      double vel = velocity(millis() - initMillis);
      if (totalInFlight() < 12) {
        launchNewPixel();
      }
      for (int c = 0; c < 8; c++) {
        for (int p = 0; p < 2; p++) {
          if (inFlight[c][p]) {
            inFlightP[c][p] += vel;
            if (inFlight[c][p] > stack[c] - 1.2) {
              stack[c] = stack[c] - 1;
              // checkCollision(c);
            }
          }
        }
      }
    }
};