#ifndef LEVEL_READER
#define LEVEL_READER
#include "application.h"
#include "Particle.h"

 class LevelReader
 {

 public:
   LevelReader();
   ~LevelReader();

   double GetPercentage();
   double GetVolume();
   void CalculateVolume(double distance);

 private:
   double mPercent;
   double mVolume;

 };

 #endif
