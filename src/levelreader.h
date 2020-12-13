#ifndef LEVEL_READER
#define LEVEL_READER
#include "application.h"
#include "Particle.h"
#include "SerialBufferRK.h"
#include "RunningAverage.h"

 class LevelReader
 {

 public:
   LevelReader(SerialBufferBase* serial);
   ~LevelReader();

   void Read();
   double GetPercentage();
   int GetVolume();
   double GetDistance();

 private:
   double SingleReading();
   bool CalculateMetrics(double distance);
   void SaveDistance(double distance);
   bool ChangeIsValid(double value, double previousValue, long previousTime, double maxRateOfChange);

 private:

   double mDistance;
   RunningAverage* mAverageVolume;
   RunningAverage* mAveragePercent;
   double mLastVolume;
   double   mLastVolTime;

   SerialBufferBase* mSerial;

 };

 #endif
