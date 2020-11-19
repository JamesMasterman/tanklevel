#include "levelreader.h"
#include "application.h"
#include "Particle.h"


LevelReader::LevelReader(SerialBufferBase* serial)
{
  mPercent = 0;
  mVolume = 0;
  mDistance = 0;
  mLastVolume = -1;
  mLastTime = 0;
  mSerial = serial;
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
  const double MaxRateOfChangeLitresPerMinute = 250;
  const double MillisPerMinute = 60000;

  double topOfWaterDistanceCm =  distance - TopOfWaterDistanceOffsetCm;
  if(topOfWaterDistanceCm < 0) topOfWaterDistanceCm = 0;
  if(topOfWaterDistanceCm > MaxHeightCm) topOfWaterDistanceCm = MaxHeightCm;

  double volume = ((MaxHeightCm - topOfWaterDistanceCm)/CmPerMetre) * PlateAreaM2 * LitresPerCum;
  if(volume < 0) volume = 0;
  if(mLastVolume < 0)
  {
    mLastVolume = volume;
  }

  double change = abs(volume - mLastVolume);
  double timeChangeMins = (millis() - mLastTime)/MillisPerMinute;
  double rateOfChangePerMin = change/timeChangeMins;

  if(rateOfChangePerMin < MaxRateOfChangeLitresPerMinute)
  {
    mPercent = (MaxHeightCm - topOfWaterDistanceCm)/MaxHeightCm  * 100.0;
    if(mPercent > 100) mPercent = 100;
    if(mPercent < 0) mPercent = 0;

    mVolume = volume;
    mLastTime = millis();
    mLastVolume = mVolume;
    return true;
  }

  return false;
}

int LevelReader::GetVolume()
{
  return (int)mVolume;
}

double LevelReader::GetPercentage()
{
  return mPercent;
}
