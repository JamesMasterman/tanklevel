#include "levelreader.h"
#include "application.h"
#include "Particle.h"

const double MaxRateOfChangeLitresPerMinute = 350;
const double MaxRateOfChangePercPerMinute = 1;
const double MillisPerMinute = 60000;

LevelReader::LevelReader(SerialBufferBase* serial)
{
  const int ReadingsToAverageCapacity = 200;
  mDistance = -1;
  mSerial = serial;
  mLastVolume = -1;
  mLastVolTime = 0;
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

  if(mDistance > 0)
  {
    //Complimentary filter on distance
    distance = mDistance * 0.5 + distance * 0.5;
  }

  CalculateMetrics(distance);
  mDistance = distance;
}

bool LevelReader::CalculateMetrics(double distance)
{
  const double TopOfWaterDistanceOffsetCm  = 25;
  const double MaxHeightCm = 200.0;
  const double PlateAreaM2 = 58.4940;
  const double LitresPerCum = 1000.0;
  const double CmPerMetre = 100.0;

  double topOfWaterDistanceCm =  distance - TopOfWaterDistanceOffsetCm;
  if(topOfWaterDistanceCm < 0) topOfWaterDistanceCm = 0;
  if(topOfWaterDistanceCm > MaxHeightCm) topOfWaterDistanceCm = MaxHeightCm;

  double volume = ((MaxHeightCm - topOfWaterDistanceCm)/CmPerMetre) * PlateAreaM2 * LitresPerCum;
  if(volume < 0) volume = 0;

  if(mLastVolume < 0)
  {
    mLastVolume = volume;
    mLastVolTime = 0;
  }

  double percent = (MaxHeightCm - topOfWaterDistanceCm)/MaxHeightCm  * 100.0;
  if(percent > 100) percent = 100;
  if(percent < 0) percent = 0;

  if(ChangeIsValid(volume, mLastVolume, mLastVolTime, MaxRateOfChangeLitresPerMinute))
  {
    mAveragePercent->addValue(percent);
    mAverageVolume->addValue(volume);
    mLastVolume = volume;
    mLastVolTime = millis();
  }

  return true;
}

int LevelReader::GetVolume()
{
  double avgVol = mAverageVolume->getAverage();
  mAverageVolume->clear();
  return (int)avgVol;
}

double LevelReader::GetPercentage()
{
  double avgPerc = mAveragePercent->getAverage();
  mAveragePercent->clear();
  return avgPerc;
}

bool LevelReader::ChangeIsValid(double value, double previousValue, long previousTime, double maxRateOfChangePerMin)
{
  const double MinTimeChangeMins = 0.0000001;
  double change = abs(value - previousValue);
  double timeChangeMins = (millis() - previousTime)/MillisPerMinute;

  double rateOfChangePerMin = 1;
  if(timeChangeMins > MinTimeChangeMins)
  {
    rateOfChangePerMin = change/timeChangeMins;
  }

  return rateOfChangePerMin <= maxRateOfChangePerMin;
}
