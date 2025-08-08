#pragma once

#include "./common.h"

const double root2 = sqrt(2) / 2.0;
const double negRoot2 = -1 * root2;

double colCoords[8][2] = {{1.0, 0.0}, {root2, root2}, {0.0, 1.0}, {negRoot2, root2}, {-1.0, 0.0}, {negRoot2, negRoot2}, {0.0, -1.0}, {root2, negRoot2}};

class TimeOval {
  public:
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

