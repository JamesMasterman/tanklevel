#ifndef LEVEL_READER
#define LEVEL_READER
#include "application.h"
#include "Particle.h"
#include "SerialBufferRK.h"

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
   void CalculateVolume();
   void SaveDistance(double distance);

 private:
   double mPercent;
   double mVolume;
   double mDistance;
   double mLastDistance;
   long   mLastTime;

   SerialBufferBase* mSerial;

 };

 #endif
