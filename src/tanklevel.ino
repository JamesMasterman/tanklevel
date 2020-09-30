#include "levellogger.h"
#include "levelreader.h"


//Thingsboard settings
const char THINGSBOARD_SERVER[] = "192.168.1.8";
const char DeviceAttributes[] = "{\"firmware_version\":\"1.4.4\",\"software_version\":\"1.0\"}";
#define THINGSBOARD_PORT 1883
#define TOKEN "ViG0BX4AZtE30aZDFnyC"

const unsigned long ONE_MIN_MS = 60*1000;
const unsigned long WATCHDOG_TIMEOUT_MS = 15*ONE_MIN_MS; //timeout for watchdog
const unsigned long LOOP_TIME_MS= 100; //sensor must be read every 100ms or no readings are obtained
const unsigned long WIFI_TIMEOUT_MS = 30*1000; //30 second timeout waiting for the wifi
const unsigned long SEND_INTERVAL_MS = 60 * ONE_MIN_MS;

STARTUP(WiFi.selectAntenna(ANT_INTERNAL));
ApplicationWatchdog wd(WATCHDOG_TIMEOUT_MS, System.reset);
SYSTEM_MODE(SEMI_AUTOMATIC);

LevelLogger* logger;
LevelReader* reader;

unsigned long lastSend;

void setup()
{
    Serial.begin(9600);
    Serial1.begin(9600);
    startWifi();
    reader = new LevelReader();
  //logger = new LevelLogger();
//  logger->Setup(THINGSBOARD_SERVER, THINGSBOARD_PORT, DeviceAttributes, TOKEN);
}

void startWifi()
{
  //Sync the time
  if(!WiFi.ready())
  {
     WiFi.on();
     WiFi.connect();
     waitFor(WiFi.ready, WIFI_TIMEOUT_MS);
  }
}

void loop()
{
  unsigned long start = millis();

  //Tell the watchdog we are still alive
  wd.checkin();

  readDistance();

  if(shouldSend())
  {
    startWifi();
    //logger->Send(reader->GetPercentage(), reader->GetVolume());
  }

  //wait till next loop
  unsigned long readDiff = millis() - start;
  delay(LOOP_TIME_MS - readDiff);

  print();
}

bool shouldSend()
{
  if(millis() - lastSend >= SEND_INTERVAL_MS)
  {
    lastSend = millis();
    return true;
  }

  return false;
}

void readDistance()
{
   //Read serial data from sensor
   float distance;
   unsigned char data[4]={};
   do
   {
      for(int i=0;i<4;i++)
      {
        data[i]=Serial1.read();
      }
   }
   while(Serial1.read()==0xff);
   Serial1.flush();

   //Calculate distance
   const float MinDistance = 280;
   if(data[0]==0xff)
   {
       int sum;
       sum=(data[0]+data[1]+data[2])&0x00FF;
       if(sum==data[3])
       {
           distance=(data[1]<<8)+data[2];
           if(distance>MinDistance)
           {
              reader->CalculateVolume((distance - MinDistance)/10.0);
           }
       }
   }
}

void print()
{
  Serial.print("Percent ");
  Serial.println(reader->GetPercentage());
  Serial.print("Volume ");
  Serial.println(reader->GetVolume());
}
