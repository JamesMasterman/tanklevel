#include "levelreader.h"
#include "application.h"
#include "Particle.h"

const double MaxRateOfChangeLitresPerMinute = 450;
const double MaxRateOfChangePercPerMinute = 1;
const double MillisPerMinute = 60000;

LevelReader::LevelReader(SerialBufferBase* serial)
{
  const int ReadingsToAverageCapacity = 100;
  mDistance = 0;
  mSerial = serial;
  mLastAverageVolume = -1;
  mLastAverageVolTime = 0;
  mLastAveragePercent = -1;
  mLastAveragePercTime = 0;
  mAverageVolume = new RunningAverage(ReadingsToAverageCapacity);
  mAveragePercent = new RunningAverage(ReadingsToAverageCapacity);

}

LevelReader::~LevelReader()
{

}

double LevelReader::GetDistance()
{
  return mDistance;
}

double LevelReader::SingleReading()
{
  //Read serial data from sensor
  float distance  = -1;

  //read out previous buffer rubbish
  while(mSerial->read() >= 0);

  //wait for the next packet
  delay(100);

  //Now read a packet
  unsigned char data[4]={};
  do
  {
     for(int i=0;i<4;i++)
     {
       data[i]=mSerial->read();
     }
  }
  while(mSerial->read()==0xff);
  mSerial->flush();

  //Calculate distance
  if(data[0]==0xff) //valid packet
  {
      int sum;
      sum=(data[0]+data[1]+data[2])&0x00FF; //checksum ok
      if(sum==data[3])
      {
        distance=(data[1]<<8)+data[2];
      }
  }

  return distance;
}

void LevelReader::Read()
{
  const int Readings = 3;
  double distance = 0;
  for(int i=0;i<Readings;i++)
  {
    double singleDist = SingleReading();
    if(singleDist > -1)
    {
      distance += singleDist;
    }else
    {
      i--;
    }
  }

  SaveDistance(distance/((double)Readings));
}

void LevelReader::SaveDistance(double distance)
{
  const float MinDistance = 290;
  const double MMtoCM = 10;

  distance = distance > MinDistance?distance:MinDistance;
  distance/= MMtoCM;

  if(CalculateVolume(distance))
  {
     mDistance = distance;
  }
}

bool LevelReader::CalculateVolume(double distance)
{
  const double TopOfWaterDistanceOffsetCm  = 30;
  const double MaxHeightCm = 200.0;
  const double PlateAreaM2 = 58.4940;
  const double LitresPerCum = 1000.0;
  const double CmPerMetre = 100.0;

  double topOfWaterDistanceCm =  distance - TopOfWaterDistanceOffsetCm;
  if(topOfWaterDistanceCm < 0) topOfWaterDistanceCm = 0;
  if(topOfWaterDistanceCm > MaxHeightCm) topOfWaterDistanceCm = MaxHeightCm;

  double volume = ((MaxHeightCm - topOfWaterDistanceCm)/CmPerMetre) * PlateAreaM2 * LitresPerCum;
  if(volume < 0) volume = 0;

  double percent = (MaxHeightCm - topOfWaterDistanceCm)/MaxHeightCm  * 100.0;
  if(percent > 100) percent = 100;
  if(percent < 0) percent = 0;

  mAveragePercent->addValue(percent);
  mAverageVolume->addValue(volume);

  return true;

}

int LevelReader::GetVolume()
{
  double avgVol = mAverageVolume->getAverage();
  mAverageVolume->clear();
  if(mLastAverageVolume < 0)
  {
    mLastAverageVolume = avgVol;
  }

  if(ChangeIsValid(avgVol, mLastAverageVolume, mLastAverageVolTime, MaxRateOfChangeLitresPerMinute))
  {
    mLastAverageVolume = avgVol;
    mLastAverageVolTime  = millis();
    return (int)avgVol;
  }

  return (int)mLastAverageVolume;
}

double LevelReader::GetPercentage()
{
  double avgPerc = mAveragePercent->getAverage();
  mAveragePercent->clear();
  if(mLastAveragePercent < 0)
  {
    mLastAveragePercent = avgPerc;
  }

  if(ChangeIsValid(avgPerc, mLastAveragePercent, mLastAveragePercTime, MaxRateOfChangePercPerMinute))
  {
    mLastAveragePercent = avgPerc;
    mLastAveragePercTime  = millis();
    return avgPerc;
  }

  return mLastAveragePercent;
}

bool LevelReader::ChangeIsValid(double value, double previousValue, long previousTime, double maxRateOfChangePerMin)
{
  double change = abs(value - previousValue);
  double timeChangeMins = (millis() - previousTime)/MillisPerMinute;
  double rateOfChangePerMin = change/timeChangeMins;

  return rateOfChangePerMin <= maxRateOfChangePerMin;
}
