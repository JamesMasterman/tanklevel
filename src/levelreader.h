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
   bool CalculateVolume(double distance);
   void SaveDistance(double distance);

 private:

   double mDistance;
   double mLastVolume;
   long   mLastTime;
   RunningAverage* mAverageVolume;
   RunningAverage* mAveragePercent;

   SerialBufferBase* mSerial;

 };

 #endif
