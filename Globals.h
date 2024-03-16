/*
 * Definition for static globals. Most of them are defined in the main .ino file.
 * This file provides `extern` references for use in implementation code.
 */

#include "ClockFaces.h"
#include "Settings.h"
#include <NTPClient.h>

extern EepromSettings eeprom_settings;

extern NTPClient time_client;
extern ace_time::TimeZone* main_time_zone;
extern bool ntp_update_occurred;
extern ace_time::CompleteZoneManager manager;

extern ClockFaces::ClockFace current_clock_face;

extern SdFs card;

extern Display display;
