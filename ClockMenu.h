/** Clock menu header. */

#pragma once

#include "defs.h"

#include "ClockFaces.h"
#include <AceTime.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "MenuClass.h"
#include "graphics.h"

extern bool ntpUpdateOccurred;
extern ClockFace curClockFace;

/**
 * Menu that can displays the clock.
 */
class ClockMenu : public Menu {
private:
    NTPClient* timing;
    ace_time::TimeZone* timezone;
    Menu* subMenu;
    int16_t updateTime = 0;
    /** Millisecond time when the last NTP update occurred*/
    int16_t timeOfLastNTP = 0;

public:
    /** The Clock menu takes a reference to the ntp client responsible for time retrieval, the time zone it should display time in, and the main menu. */
    ClockMenu(NTPClient* timing, ace_time::TimeZone* tz, Menu* mainMenu);
    Menu* drawMenu(T_DISPLAY* disp, uint16_t deltaMillis) override;
    bool shouldRefresh(uint16_t deltaMillis) override;
    Menu* handleButton(uint8_t buttons) override;
};
