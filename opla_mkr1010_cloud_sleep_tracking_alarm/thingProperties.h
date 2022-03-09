#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#include "arduino_secrets.h"

void onSwitchAlarmChange();
void onAlarmTimeChange();
void onSleepTimeChange();

bool switchAlarm = false;
CloudTime alarmTime;
int sleepTime;

void initProperties() {

  ArduinoCloud.setThingId(THING_ID);
#if defined(BOARD_HAS_WIFI) || defined(BOARD_HAS_GSM) || defined(BOARD_HAS_NB)
  ArduinoCloud.addProperty(switchAlarm, READWRITE, ON_CHANGE, onSwitchAlarmChange);
  ArduinoCloud.addProperty(alarmTime, READWRITE, ON_CHANGE, onAlarmTimeChange);
  ArduinoCloud.addProperty(sleepTime, READWRITE, ON_CHANGE, onSleepTimeChange);
#endif
}

#if defined(BOARD_HAS_WIFI)
  WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_SSID, SECRET_PASS);
#endif
