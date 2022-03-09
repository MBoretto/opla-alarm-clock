#ifndef SCREEN_CONTROLLER
#define SCREEN_CONTROLLER

#include <RTCZero.h>
#include <Arduino_MKRIoTCarrier.h>

#include "images.h"

class ScreenController {
  public:
    int GMT; //change this to adapt it to your time zone
    bool is_wifi_connected;
    bool is_time_set;

  ScreenController(MKRIoTCarrier* carrier, RTCZero* rtc, int gmt):
    GMT(gmt),
    m_carrier(carrier),
    m_rtc(rtc),
    is_first_time_render(true),
    current_color(ST77XX_WHITE),
    screen_status_second(0),
    is_wifi_connected(false), // allows to do the first render
    screen_status_is_wifi_connected(true),
    is_time_set(false) {
  }

  bool isFirstRender() const {
    return is_first_time_render;
  }


  void off() const  {
    m_carrier->display.enableDisplay(false);
  }

  void on() const {
    m_carrier->display.enableDisplay(true);
  }
  void cleanScreen() const {
    m_carrier->display.fillScreen(ST77XX_BLACK); //black background
  }

  void render() {
    on();
    if (is_time_set) {
      if (isFirstRender()) {
        initRender();
      }
      uint16_t second = m_rtc->getSeconds();

      //printint everything until 0
      if (second < (screen_status_second + 1)) {
        for (uint16_t this_second = (screen_status_second + 1); this_second <= 59; this_second++) {
          printBorderSeconds(this_second);
        }
        printBorderSeconds(0);
        printTime();
        screen_status_second = 0;
      }
      for (uint16_t this_second = (screen_status_second + 1); this_second <= second; this_second++) {
        if (printBorderSeconds(this_second)) {
          printTime();
        }
      }
      screen_status_second = second;
    }

    drawWifiIcon();

    if (screen_status_is_wifi_connected and not is_time_set) {
      printDateUnavailable();
    } else if (screen_status_is_wifi_connected and is_time_set) {
      cleanMessage();
    }
  }

  void cleanMessage() {
     printMessage("                ");
  }

  void printDateUnavailable() {
    printMessage("Fetching date..");
  }

  void drawWifiIcon() {
    if (screen_status_is_wifi_connected == is_wifi_connected) {
      // the screen already reflects the status no need to update.
      return;
    }
    screen_status_is_wifi_connected = is_wifi_connected;

    cleanMessage();
    m_carrier->display.drawBitmap(110, 200, clean, 15, 15, ST77XX_BLACK);
    if (is_wifi_connected) {
      m_carrier->display.drawBitmap(110, 200, wifi, 15, 15, ST77XX_WHITE);
      return;
    }
    printConnectingWiFI();
    m_carrier->display.drawBitmap(110, 200, nowifi, 15, 15, ST77XX_RED);
  }


  void printAlarm(bool alarm_is_enabled) {
    if (alarm_is_enabled) {
        m_carrier->display.drawBitmap(110, 30, alarmon, 15, 15, ST77XX_RED);
        return;
    }
    m_carrier->display.drawBitmap(110, 30, clean, 15, 15, ST77XX_BLACK);
  }

  void printAlarmTime(uint alarm_time_second) {
    uint raw_minutes = alarm_time_second / 60;
    uint clock_hour = raw_minutes / 60;
    uint clock_minute = raw_minutes % 60;
    m_carrier->display.setTextColor(ST77XX_RED, ST77XX_BLACK);
    m_carrier->display.setCursor(87, 55); //sets position for printing (x and y)
    m_carrier->display.setTextSize(2);
    if (clock_hour < 10) {
      m_carrier->display.print(" ");
    }
    m_carrier->display.print(clock_hour);
    m_carrier->display.print(":");
    if (clock_minute < 10) {
      m_carrier->display.print("0");
    }
    m_carrier->display.print(clock_minute);
  }

  private:
  bool is_first_time_render;
  bool screen_status_is_wifi_connected;
  uint16_t current_color;
  uint16_t screen_status_second;
  //uint32_t alarm_time_second;

  MKRIoTCarrier* m_carrier;
  RTCZero* m_rtc;

  void printMessage(String message) {
    m_carrier->display.setTextColor(ST77XX_WHITE, ST77XX_BLACK); //white text
    m_carrier->display.setCursor(75, 155); //sets position for printing (x and y)
    m_carrier->display.setTextSize(1);
    m_carrier->display.print(message);
  }


  void printConnectingWiFI() {
    printMessage("  Connecting..");
  }

  void initRender() {
    printTime();
    is_first_time_render = false;
  }

  bool printBorderSeconds(uint16_t second) {
    const float pi = 3.14;
    const float pi_mezzi = pi / 2;
    //Serial.print("printing: ");
    //Serial.println(second);
    const float step = 2 * pi / 60;
    const float start = (second - 1) * step - pi_mezzi;
    const float stop = second * step - pi_mezzi;

    //for (float rad = start; rad < stop; rad = rad + 0.1) {
    //for (float rad = start; rad < stop; rad = rad + 0.05) {
    for (float rad = start; rad < stop; rad = rad + 0.01) {
      for (uint16_t radius = 110; radius <= 117; radius++) {
        const uint16_t x = 120 + radius * cos(rad);
        const uint16_t y = 120 + radius * sin(rad);
        //Serial.print(x);
        //Serial.print(" ");
        //Serial.println(y);
        m_carrier->display.drawPixel(x, y, current_color);
      }
    }
    if (second == 0) {
      if (current_color == ST77XX_WHITE) {
        current_color = ST77XX_BLACK;
      } else {
        current_color = ST77XX_WHITE;
      }
      return true;
   }
   return false;
  }

  void printTime() {
    m_carrier->display.setTextColor(ST77XX_WHITE, ST77XX_BLACK); //white text black Background
    m_carrier->display.setTextSize(6); //medium sized text

    m_carrier->display.setCursor(30, 100); //sets position for printing (x and y)
    uint16_t hour = m_rtc->getHours() + GMT;
    if (hour > 23) {
      // Deal with impossible hours intoduced by GTM conversion
      m_carrier->display.print(hour - 24);
    } else {
      if (hour < 10) {
        m_carrier->display.print(" ");
      }
       m_carrier->display.print(hour);
    }
    m_carrier->display.print(":");
    if (m_rtc->getMinutes() < 10) {
      m_carrier->display.print("0");
    }
    m_carrier->display.print(m_rtc->getMinutes());
  }
};
#endif
