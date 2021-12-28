#include "levellogger.h"


LevelLogger::LevelLogger()
{

}

LevelLogger::~LevelLogger()
{

}

void LevelLogger::Send(double percentage, double volume)
{
    if(EnsureConnected())
    {
      SendData(percentage, volume);
    }
}

bool LevelLogger::SendData(double percentage, double volume)
{
    // Prepare JSON payload
    snprintf(m_mqttPub, sizeof(m_mqttPub), "{\"percentage\":%.1f,\"tankvolume\":%.1f}", percentage, volume);
    //Serial.println(m_mqttPub);

    // Send payload
    bool result = psClient->publish("v1/devices/me/telemetry", m_mqttPub);            // Topic, payload
    result = psClient->loop() && result;
    BlinkLED();
    return result;
}
