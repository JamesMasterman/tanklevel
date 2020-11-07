#include "levelreader.h"
#include "application.h"
#include "Particle.h"


LevelReader::LevelReader(SerialBufferBase* serial)
{
  mPercent = 0;
  mVolume = 0;
  mDistance = 0;
  mLastDistance = -1;
  mOutOfPhaseReadingCount = 0;
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
  const double Tolerance = 0.05;
  const int MaxOutOfPhase = 200;
  const float MinDistance = 290;
  const double MMtoCM = 10;

  mDistance = distance > MinDistance?distance:MinDistance;
  mDistance/= MMtoCM;
  if(mOutOfPhaseReadingCount > MaxOutOfPhase)
  {
    mOutOfPhaseReadingCount=0;
    mLastDistance = mDistance;
  }

  double change = abs(mDistance - mLastDistance);
  double allowableChange = mLastDistance * Tolerance;
  if(mLastDistance > -1 && change > allowableChange)
  {
     mDistance = mLastDistance;
     mOutOfPhaseReadingCount++;
  }
  else
  {
    CalculateVolume();
    mLastDistance = mDistance;
  }
}

void LevelReader::CalculateVolume()
{
  const double TopOfWaterDistanceOffsetCm  = 30;
  const double MaxHeightCm = 200.0;
  const double PlateAreaM2 = 58.4940;
  const double LitresPerCum = 1000.0;
  const double CmPerMetre = 100.0;
  double topOfWaterDistanceCm =  mDistance - TopOfWaterDistanceOffsetCm;
  if(topOfWaterDistanceCm < 0) topOfWaterDistanceCm = 0;
  if(topOfWaterDistanceCm > MaxHeightCm) topOfWaterDistanceCm = MaxHeightCm;

  mPercent = (MaxHeightCm - topOfWaterDistanceCm)/MaxHeightCm  * 100.0;
  mVolume = ((MaxHeightCm - topOfWaterDistanceCm)/CmPerMetre) * PlateAreaM2 * LitresPerCum;

  if(mPercent > 100) mPercent = 100;
  if(mPercent < 0) mPercent = 0;
  if(mVolume < 0) mVolume = 0;
}

int LevelReader::GetVolume()
{
  return (int)mVolume;
}

double LevelReader::GetPercentage()
{
  return mPercent;
}
