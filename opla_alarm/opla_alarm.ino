#include "ScreenController.hpp"

#include "arduino_secrets.h"
#include "thingProperties.h"

MKRIoTCarrier carrier;
RTCZero rtclock;
const int GMT = +1;
ScreenController screen(&carrier, &rtclock, GMT);

// Check the connection every 15 seconds
const long interval = 15 * 1000;
unsigned long previous_millis = 0;
bool wifi_first_connection = true;

const long ntp_interval = 60000 * 5; //5 minutes

unsigned long ntp_previous_millis = 0;

const long date_interval = 1000; //every second
unsigned long date_previous_millis = 0;

bool is_ntp_time_sync = false;

void setup() {
  /* Initialize serial and wait up to 5 seconds for port to open */
  Serial.begin(115200);
  for(unsigned long const serialBeginTime = millis(); !Serial && (millis() - serialBeginTime > 5000);) {
    continue;
  }

  /* This function takes care of connecting your sketch variables to the ArduinoIoTCloud object */
  initProperties();

  /* Initialize Arduino IoT Cloud library */
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  //setDebugMessageLevel(DBG_INFO);
  setDebugMessageLevel(DBG_DEBUG);
  ArduinoCloud.printDebugInfo();
  // ArduinoCloud.updateInternalTimezoneInfo();

  rtclock.begin(true); //resetTime = true
  //Serial.println(TOUCH.getSensorsSensitivity());
  CARRIER_CASE = true;
  carrier.begin();
 
  TOUCH.setSensorsSensitivity(100);
  Serial.print("Button sensitivity:");
  Serial.println(TOUCH.getSensorsSensitivity());
}

unsigned long prev_sound = 0;
volatile bool play_alarm = false;

void loop() {
  ArduinoCloud.update();
  const unsigned long current_millis = millis();

  if ((current_millis - date_previous_millis) >= date_interval) {
    date_previous_millis = current_millis;  
    screen.render();
  }
    
  if (play_alarm) {
    if ((current_millis - prev_sound > 1000)) {
      prev_sound = current_millis;
      tone(carrier.Buzzer.getPin(), 500, 500);// async
    }
  }
  //Serial.println(TOUCH.getSensorsSensitivity());
  carrier.Buttons.update();
  if (carrier.Buttons.onTouchUp(TOUCH0) or
      carrier.Buttons.onTouchUp(TOUCH1) or
      carrier.Buttons.onTouchUp(TOUCH2) or
      carrier.Buttons.onTouchUp(TOUCH3) or
      carrier.Buttons.onTouchUp(TOUCH4)
     ) {
    Serial.println("Touched Down a button");
    play_alarm = false;
  }

  // Checking the network status every X seconds
  if (current_millis - previous_millis >= interval or wifi_first_connection) {
    previous_millis = current_millis;
    if (ArduinoCloud.connected() != 0) {
      // Wifi Ok
      screen.is_wifi_connected = true;

      // I'm sure that the WIFI is active I sync the time with NTP Protocol
      if (current_millis - ntp_previous_millis >= ntp_interval or not is_ntp_time_sync) {
        uint32_t epoch = 0;
        uint8_t attempts = 0;
                
        while (epoch == 0) {
          epoch = ArduinoCloud.getInternalTime();
          // Limit
          if (++attempts > 4) {
            break;
          }
        }
        if (epoch == 0) {
          Serial.println("NTP unreachable!");
        } else {
          screen.is_time_set = true;
          if (is_ntp_time_sync) {
            // Checking if the internal RTC is still sync
            uint32_t rtc_epoch = rtclock.getEpoch();
            if (rtc_epoch != epoch) {
              Serial.print("Internal epoch NOT sync anymore: rtc_epoch != epoch ");
              Serial.print(rtc_epoch);
              Serial.print(" != ");
              Serial.println(epoch);
              is_ntp_time_sync = false;
            } else {
              Serial.print("Internal epoch still sync at ");
              printRTCInternalDate();
            }
          }
          // First epoch set or out of sync..
          if (not is_ntp_time_sync) {
            rtclock.setEpoch(epoch);
            is_ntp_time_sync = true; //TODO check if the time is still on sync!
            Serial.print("Epoch set: ");
            Serial.println(epoch);
            Serial.print("Internal RTC time: ");
            printRTCInternalDate();
          }
          ntp_previous_millis = current_millis; // Force NTP check every n minutes
        }
      }
    } else {
      screen.is_wifi_connected = false;
    }
  }
}

void setAlarm() {
  rtclock.disableAlarm();
  rtclock.detachInterrupt();  
  if (switchAlarm) {
    uint alarm_time_second = alarmTime;
    uint raw_minutes = alarm_time_second / 60;
    uint8_t clock_hour = raw_minutes / 60 - GMT;
    uint8_t clock_minute = raw_minutes % 60;
    if (clock_hour >= 24) {
      clock_hour -= 24;
    } else if (clock_hour < 0) {
      clock_hour += 24;
    }
     
    rtclock.setAlarmTime(clock_hour, clock_minute, 0);
    rtclock.enableAlarm(rtclock.MATCH_HHMMSS);
    
    rtclock.attachInterrupt(alarmMatch);
    Serial.println("Alarm set");
    return;
  }
  Serial.println("Alarm UNset");
}

void alarmMatch() {
  Serial.println("Interrupt triggered");
  play_alarm = true;
}

void onAlarmTimeChange() {
   setAlarm();
   Serial.print("Alarm changed: ");
   Serial.println(alarmTime);
//   uint raw_minutes = alarmTime / 60;
//   uint clock_hour = raw_minutes / 60;
//   uint clock_minute = raw_minutes % 60;
//   Serial.print(clock_hour);
//   Serial.print(":");
//   Serial.println(clock_minute);
   screen.printAlarmTime(alarmTime);
}

void onSwitchAlarmChange() {
  setAlarm();
  Serial.print("Switching alarm status to: ");
  Serial.print(switchAlarm);
  screen.printAlarm(switchAlarm);
}

void printRTCInternalDate() {
  Serial.print(rtclock.getDay());
  Serial.print("/");
  Serial.print(rtclock.getMonth());
  Serial.print("/");
  Serial.print(rtclock.getYear());
  Serial.print(" ");
  Serial.print(rtclock.getHours() + GMT);
  Serial.print(":");
  Serial.print(rtclock.getMinutes());
  Serial.print(":");
  Serial.print(rtclock.getSeconds());
  Serial.println(" ");
}
