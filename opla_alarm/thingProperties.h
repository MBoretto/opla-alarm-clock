#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#include "arduino_secrets.h"

void onSwitchAlarmChange();
void onAlarmTimeChange();

bool switchAlarm;
CloudTime alarmTime;

void initProperties() {

  ArduinoCloud.setThingId(THING_ID);
#if defined(BOARD_HAS_WIFI) || defined(BOARD_HAS_GSM) || defined(BOARD_HAS_NB)
  ArduinoCloud.addProperty(switchAlarm, READWRITE, ON_CHANGE, onSwitchAlarmChange);
  ArduinoCloud.addProperty(alarmTime, READWRITE, ON_CHANGE, onAlarmTimeChange);
#endif
}

#if defined(BOARD_HAS_WIFI)
  WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_SSID, SECRET_PASS);
#endif
