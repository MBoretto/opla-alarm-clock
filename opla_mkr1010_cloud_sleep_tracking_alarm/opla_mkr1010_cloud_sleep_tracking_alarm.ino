#include <ScreenController.hpp>
#include <DateTime.hpp>

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


enum Status {MODE_CLOCK, MODE_BEDTIME, MODE_SLEEP, MODE_ALARM};
volatile Status status = MODE_CLOCK;

unsigned long prev_sound = 0;
uint32_t mode_sleep_started_at = 0;

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

void loop() {
  ArduinoCloud.update();
  const unsigned long current_millis = millis();

  if ((current_millis - date_previous_millis) >= date_interval) {
    date_previous_millis = current_millis;
    if (status == MODE_SLEEP) {
      // switch off the screen
      carrier.display.enableDisplay(false);
      screen.off();
      
    } else {
       screen.render();
    }
   

    bool is_bedtime = false;
    if (switchAlarm and status == MODE_CLOCK) { // Checking only if  the alarm is enabled
      uint8_t hours_GTM = rtclock.getHours() + GMT;
      if (hours_GTM > 23) {
        hours_GTM -= 24;
      }
      DateTime current_time(hours_GTM, rtclock.getMinutes()); // RTC time has no timezone (+0)
      DateTime alarm(alarmTime); // This is the time entered in the IOT cloud is consistent with your  timezone
      DateTime bedtime_start = alarm - (8 * 60 * 60 + 30 * 60); // sleep for 8:30 hours
      if (bedtime_start < alarm) {
        // interval between bedtime and alarm do not cross midnight
        if (bedtime_start < current_time and current_time < alarm) {
          is_bedtime = true;
        }
      } else {
        if (
          (bedtime_start < current_time and current_time < DateTime(23, 59)) or
          (DateTime(0, 0) < current_time and current_time < alarm) 
          ) {
          is_bedtime = true;
        }
      }
 
      if (is_bedtime) {
        status = MODE_BEDTIME;
        Serial.println("Sleep mode active");
        for (uint8_t led_index = 0; led_index < 5; ++led_index) { 
          carrier.leds.setPixelColor(led_index, 20 , 20 , 20);     
        }
        
        //carrier.leds.setBrightness(50);
        carrier.leds.show();
      }
    }
  }
    
  if (status == MODE_ALARM) {
    // play the alarm
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
      if (status == MODE_BEDTIME) {
        status = MODE_SLEEP;
        mode_sleep_started_at = rtclock.getEpoch(); // Not timezone!
        for (uint8_t led_index = 0; led_index < 5; ++led_index) { 
          carrier.leds.setPixelColor(led_index,  0,  0 , 0);
        }
        carrier.leds.show();
        Serial.println("Sleep mode active");
      } else if (status == MODE_ALARM) {
         status = MODE_CLOCK;
         sleepTime = (rtclock.getEpoch() - mode_sleep_started_at) / 60; // Updating the sleep time on the cloud
      }
  }


  // Checking the network status every X seconds
  if (current_millis - previous_millis >= interval or wifi_first_connection) {
    previous_millis = current_millis;
    if (ArduinoCloud.connected() != 0) {
      // Wifi Ok
      screen.is_wifi_connected = true;
      
      //printWiFiStatus();
      //if (is_ntp_time_sync) {
      //  Serial.print("Internal RTC time: ");
      //  printRTCInternalDate();
      //}

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
  
  status = MODE_ALARM;
}

void onAlarmTimeChange() {
   setAlarm();
   Serial.print("Alarm changed: ");
   Serial.println(alarmTime);
   screen.printAlarmTime(alarmTime);
}

void onSwitchAlarmChange() {
  setAlarm();
  Serial.print("Switching alarm status to: ");
  Serial.print(switchAlarm);
  screen.printAlarm(switchAlarm);
}

void onSleepTimeChange() {
  // Do nothing
}


// TODO pointer
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
