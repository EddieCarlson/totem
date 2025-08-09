#pragma once

#include "./common.h"

class ColorPixel {
  public:
    CRGB color;
    int col;
    double row;
    bool active;
    bool stacked;

    ColorPixel() {
        reset();
    }

    void reset() {
        color = CRGB::Black;
        col = -1;
        row = -1;
        active = false;
        stacked = false;
    }

    void activate(int _col, CRGB _color) {
        active = true;
        col = _col;
        color = _color;
        row = 0.1;
    }

    void advance(double rows) {
        row += rows;
    }

    void setStacked() {
        stacked = true;
        active = false;
    }

    int closestRow() {
        return round(row);
    }
};

class TetrisLite {
  public:
    int stack[8]; // bottom row of stacked static blocks at the top of the rod in each col (starts at NUM_LEDS - i.e. off the rod)
    int nextCol = 0;
    ColorPixel* inFlightP[24];
    bool inFlight[8];
    CRGB colors[4];
    uint32_t lastLaunched = 0;
    double velocity;
    double interval;
    uint32_t lastUpdate;
    CRGB stackColors[8][NUM_ROWS];

    TetrisLite(CRGB* _colors, double _velocity) {
        velocity = _velocity;
        interval = 30;
        for (int i = 0; i < 4; i++) {
            colors[i] = _colors[i];
        }
        for (int c = 0; c < 24; c++) {
            inFlightP[c] = new ColorPixel();
        }
        resetStack();
    }

    void resetStack() {
        for (int i = 0; i < 24; i++) {
            inFlightP[i]->reset();
        }
        for (int c = 0; c < 8; c++) {
            stack[c] = topRowIdxForCol(c) + 1;
            inFlight[c] = false;
            for(int r = 0; r < NUM_ROWS; r++) {
                stackColors[c][r] = CRGB::Black;
            }
        }
        lastUpdate = millis();
    }

    int getNextCol() {
        uint32_t t = millis();
        if (t - lastLaunched > interval) {
            int nextCol = (int) (rand01() * 8);
            // while (inFlightP[nextCol]->active) {
            //     nextCol = (int) (rand01() * 8);
            // }
            lastLaunched = t;
            return nextCol;
            // int c = nextCol;
            // nextCol = (nextCol + 1) % 8;
            // return c;
        } else {
            return -1;
        }
    }

    void launchPixel() {
        int c = getNextCol();
        if (c >= 0) {
            CRGB color = colors[(int) (rand01() * 4)];
            inFlightP[c]->activate(c, color);
        }
    }

    void movePixels() {
        for (int i = 0; i < 24; i++) {
            ColorPixel* cp = inFlightP[i];
            if (cp->active) {
                double rows = velocity * ((double) (millis() - lastUpdate)) / 1000.0;
                cp->advance(rows);
                if (cp->row >= stack[cp->col] - 1.1) {
                    stack[cp->col] = stack[cp->col] - 1;
                    CRGB c = cp->color;
                    stackColors[cp->col][stack[cp->col]] = cp->color;
                    stackColors[cp->col][stack[cp->col]] = stackColors[cp->col][stack[cp->col]].nscale8(0.15 * 256);
                    // setPixel(cp->col, stack[cp->col], cp->color, 0.1);
                    cp->reset();
                }
            }
        }
    }

    void update() {
        launchPixel();
        movePixels();
        lastUpdate = millis();
        for (int c = 0; c < 8; c++) {
            // int topOfStack = stack[c];
            for (int r = 0; r < NUM_ROWS; r++) {
                setPixel(c, r, stackColors[c][r]);
            }
        }
        for (int c = 0; c < 24; c++) {
            ColorPixel* cp = inFlightP[c];
            if (cp->active) {
                setPixel(c, cp->closestRow(), cp->color, 0.15);
            }
        }
    }
};

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