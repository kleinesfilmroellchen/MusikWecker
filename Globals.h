/*
 * Definition for static globals. Most of them are defined in the main .ino file.
 * This file provides `extern` references for use in implementation code.
 */

#include "ClockFaces.h"
#include "Settings.h"
#include <ESP8266WiFi.h>
#include <NTPClient.h>

extern EepromSettings eeprom_settings;

extern ClockFaces::ClockFace current_clock_face;

extern SdFs card;

extern Display display;
