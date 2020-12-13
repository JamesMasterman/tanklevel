//
//    FILE: RunningAverage.h
//  AUTHOR: Rob dot Tillaart at gmail dot com
// VERSION: 0.2.06
//    DATE: 2015-mar-07
// PURPOSE: RunningAverage library for Arduino
//     URL: http://arduino.cc/playground/Main/RunningAverage
// HISTORY: See RunningAverage.cpp
//
// Released to the public domain
//
// backwards compatibility
// clr() clear()
// add(x) addValue(x)
// avg() getAverage()
#ifndef RunningAverage_h
#define RunningAverage_h

#define RUNNINGAVERAGE_LIB_VERSION "0.2.06"

//#include "Arduino.h"
//Arduino removed and Application added for spark.io compatibility
#include "application.h"

class RunningAverage
{
public:
    RunningAverage(void);
    RunningAverage(uint32_t);
    ~RunningAverage();

    void clear();
    void addValue(double);
    void fillValue(double, uint32_t);

    double getAverage();

    double getElement(uint32_t idx);
    uint8_t getSize() { return _size; }
    uint8_t getCount() { return _cnt; }

protected:
    uint32_t _size;
    uint32_t _cnt;
    uint32_t _idx;
    double   _sum;
    double * _ar;
};

#endif
