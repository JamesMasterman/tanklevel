#ifndef FLOW_LOGGER
#define FLOW_LOGGER

#include <stdint.h>
#include "application.h"
#include <PubSubClient.h>
#include "mqttlogger.h"

class LevelLogger: public MQTTLogger
{

public:
  LevelLogger();
  ~LevelLogger();

public:
  void Send(double percentage, double volume);

private:
  bool SendData(double percentage, double volume);
};

#endif
