#include <WiFiNINA.h>
#include <ScreenController.hpp>

#include "arduino_secrets.h"

const char ssid[] = SECRET_SSID;        // your network SSID (name)
const char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)



MKRIoTCarrier carrier;
RTCZero rtc;

int status = WL_IDLE_STATUS;     // the Wifi radio's status
const int GMT = +1;

ScreenController screen(&carrier, &rtc, GMT);

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
  // Serial port for debugging purposes
  Serial.begin(115200);

  rtc.begin(true); //resetTime = true

  CARRIER_CASE = true;
  carrier.begin();
}

void loop() {
  const unsigned long current_millis = millis();

  if ((current_millis - date_previous_millis) >= date_interval) {
    date_previous_millis = current_millis;
    //status = WiFi.status();
    if (isWifiConnected()) {
      screen.is_wifi_connected = true;
    } else {
      screen.is_wifi_connected = false;
    }    
    screen.render();
  }


  // Checking the network status every X seconds
  if (current_millis - previous_millis >= interval or wifi_first_connection) {
    previous_millis = current_millis;
    status = WiFi.status();
    if (isWifiConnected()) {
      // Wifi Ok
     
      
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
          epoch = WiFi.getTime();
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
            uint32_t rtc_epoch = rtc.getEpoch();
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
            rtc.setEpoch(epoch);
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
      //screen.is_wifi_connected = false;
      Serial.print("Wifi status: ");
      Serial.println(status);
      if (wifi_first_connection) {
        Serial.print("Attempting to connect to network: ");
      } else {
        Serial.print("Attempting to re-connect to network: ");
      }
      Serial.println(ssid);

      // Connect to WPA/WPA2 network returns the status
      status = WiFi.begin(ssid, pass);
      if (isWifiConnected()) {
        //screen.is_wifi_connected = true;
        wifi_first_connection = false;  
        printWiFiStatus();
      }
    }
  }
  

}


void printRTCInternalDate() {
  Serial.print(rtc.getDay());
  Serial.print("/");
  Serial.print(rtc.getMonth());
  Serial.print("/");
  Serial.print(rtc.getYear());
  Serial.print(" ");
  Serial.print(rtc.getHours() + GMT);
  Serial.print(":");
  Serial.print(rtc.getMinutes());
  Serial.print(":");
  Serial.print(rtc.getSeconds());
  Serial.println(" ");
}

bool isWifiConnected() {
  return status == WL_CONNECTED;
}

void printWiFiStatus() {
  Serial.println("\n----------------------------------------");
  
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  IPAddress subney_mask = WiFi.subnetMask();
  IPAddress gateway_ip = WiFi.gatewayIP();
  Serial.print("Board IP Address: ");
  Serial.print(ip);
  Serial.print(" Subnet mask: ");
  Serial.print(subney_mask);
  Serial.print(" gateway: ");
  Serial.println(gateway_ip);
  Serial.print("Network ");
  Serial.print("SSID: ");
  Serial.print(WiFi.SSID());
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(" signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}
