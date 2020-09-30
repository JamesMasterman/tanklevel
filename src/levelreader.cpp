#include "levelreader.h"
#include "application.h"
#include "Particle.h"


LevelReader::LevelReader()
{
  mPercent = 0;
  mVolume = 0;
}

LevelReader::~LevelReader()
{

}

void LevelReader::CalculateVolume(double distance)
{
  const double MaxHeightCm = 200.0;
  const double PlateAreaM2 = 56.745;
  const double LitresPerCum = 1000.0;
  const double CmPerMetre = 100.0;
  mPercent = (MaxHeightCm - distance)/MaxHeightCm  * 100.0;
  mVolume = ((MaxHeightCm - distance)/CmPerMetre) * PlateAreaM2 * LitresPerCum;

  if(mPercent > 100) mPercent = 100;
  if(mPercent < 0) mPercent = 0;
  if(mVolume < 0) mVolume = 0;
}

double LevelReader::GetVolume()
{
  return mVolume;
}

double LevelReader::GetPercentage()
{
  return mPercent;
}
