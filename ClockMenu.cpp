#include "ClockMenu.h"
#include "Audio.h"
#include "Settings.h"
#include <NTPClient.h>

extern eeprom_settings_struct eeprom_settings;

ClockMenu::ClockMenu(NTPClient* timing, ace_time::TimeZone* tz, Menu* mainMenu)
{
    this->timing = timing;
    this->timezone = tz;
    this->parent = this;
    this->subMenu = mainMenu;
    mainMenu->parent = this;
}

Menu* ClockMenu::drawMenu(T_DISPLAY* disp, uint16_t deltaMillis)
{
    // Clock menu consists of
    // - clock face determined by clock face function
    // - Status symbols above
    // - Date below

    char dateStr[11];
    ace_time::ZonedDateTime curtime = ace_time::ZonedDateTime::forUnixSeconds64(this->timing->getEpochTime(), *this->timezone);
    sprintf(dateStr, "%02u.%02u.%4u", curtime.day(), curtime.month(), curtime.year());

    disp->firstPage();
    do {
        // i unnecessarily optimized the sin and cos into a lut, took me three hours.
        // this was the fix, i wasn't resetting the drawing mode back to normal (white)
        // fuck my life
        disp->setDrawColor(1);
        volatile uint16_t symbolpos = SCREEN_WIDTH;
        // status symbols
        if (WiFi.status() == WL_CONNECTED) {
            disp->drawXBM(symbolpos - wifi_symbol_width, 0, wifi_symbol_width, wifi_symbol_height, wifi_symbol_bits);
            symbolpos -= wifi_symbol_width + SYMBOL_SPACING;
        } else {
            disp->drawXBM(symbolpos - nowifi_symbol_width, 0, nowifi_symbol_width, nowifi_symbol_height, nowifi_symbol_bits);
            symbolpos -= nowifi_symbol_width + SYMBOL_SPACING;
        }
        audioLoop();

        // TODO: display alarm clock symbol if an alarm clock is set

        if (this->timeOfLastNTP >= 0) {
            disp->drawXBM(symbolpos - clocksync_symbol_width, 0, clocksync_symbol_width, clocksync_symbol_height, clocksync_symbol_bits);
            symbolpos -= clocksync_symbol_width + SYMBOL_SPACING;
        }
        audioLoop();

        if (audioPlayer && audioPlayer->isRunning()) {
            disp->drawXBM(symbolpos - sound_symbol_width, 0, sound_symbol_width, sound_symbol_height, sound_symbol_bits);
            symbolpos -= sound_symbol_width + SYMBOL_SPACING;
        }
        audioLoop();

        volatile uint64_t time = micros64();
        curClockFace(disp, &curtime, 0, 0, disp->getDisplayWidth(), disp->getDisplayHeight());
        disp->setFont(TINY_FONT);
        disp->drawUTF8(LEFT_TEXT_MARGIN, SCREEN_HEIGHT, dateStr);
        volatile uint64_t after_time = micros64();
        audioLoop();

        if (eeprom_settings.show_timing) {
            volatile uint64_t total_time = after_time - time;
            volatile uint32_t cycles = microsecondsToClockCycles(total_time);
            char timingStr[22];
            sprintf(timingStr, "%02.3fm %10d", total_time / 1000.0d, cycles);

            disp->drawUTF8(SCREEN_WIDTH - LEFT_TEXT_MARGIN - 70, SCREEN_HEIGHT, timingStr);
        }

        audioLoop();
    } while (disp->nextPage());

    return this;
}

bool ClockMenu::shouldRefresh(uint16_t deltaMillis)
{
    // set ntp sync display timer, if necessary
    if (ntpUpdateOccurred)
        this->timeOfLastNTP = CLOCKSYNC_SYMBOL_DURATION;

    if (this->timeOfLastNTP >= 0)
        this->timeOfLastNTP = this->timeOfLastNTP - deltaMillis;

    this->updateTime += deltaMillis;
    //  Serial.printf("ut %dms, dt %dms, ntp %dms\n", this->updateTime, deltaMillis, this->timeOfLastNTP);
    if (this->updateTime > CLOCK_UPDATE_INTERVAL) {
        this->updateTime = 0;
        return true;
    }
    return false;
}
Menu* ClockMenu::handleButton(uint8_t buttons)
{
    if (buttons & B_RIGHT) {
        return this->subMenu;
    }
    return this;
}
