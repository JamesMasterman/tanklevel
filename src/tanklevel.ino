#include "levellogger.h"
#include "levelreader.h"
#include "SerialBufferRK.h"

//Thingsboard settings
const char THINGSBOARD_SERVER[] = "192.168.1.8";
const char DeviceAttributes[] = "{\"firmware_version\":\"1.5.2\",\"software_version\":\"1.2\"}";
#define THINGSBOARD_PORT 1883
#define TOKEN "9sJaRrySzwBn3QgkshnB"

const unsigned long ONE_MIN_MS = 60*1000;
const unsigned long WATCHDOG_TIMEOUT_MS = 15*ONE_MIN_MS; //timeout for watchdog
const unsigned long LOOP_TIME_MS= 1000;
const unsigned long SEND_INTERVAL_MS = 2*ONE_MIN_MS;

STARTUP(WiFi.selectAntenna(ANT_INTERNAL));
ApplicationWatchdog wd(WATCHDOG_TIMEOUT_MS, System.reset);
SYSTEM_THREAD(ENABLED);

LevelLogger* logger;
LevelReader* reader;
SerialBuffer<4096> serial1Buffer(Serial1);

unsigned long lastSend = 0;

void setup()
{
    //Serial.begin(9600);
    logger = new LevelLogger();
    logger->Setup(THINGSBOARD_SERVER, THINGSBOARD_PORT, DeviceAttributes, TOKEN);

    Serial1.begin(9600, SERIAL_8N1);
    serial1Buffer.setup();
    reader = new LevelReader(&serial1Buffer);
}

void loop()
{

  //Tell the watchdog we are still alive
  wd.checkin();
  reader->Read();

  if(shouldSend())
  {
    logger->Send(reader->GetPercentage(), reader->GetVolume());
  }

  //wait till next loop
  delay(LOOP_TIME_MS);

  //print();
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


void print()
{
  Serial.print("Distance ");
  Serial.println(reader->GetDistance());
  Serial.print("Percent ");
  Serial.println(reader->GetPercentage());
  Serial.print("Volume ");
  Serial.println(reader->GetVolume());
}
