/** Defines the eeprom_settings struct which declares the setting data structure in (emulated) EEPROM. */
#pragma once

#include "defs.h"
#include <Arduino.h>
#include <EEPROM.h>

/**
   The settings store all user customizable data except wifi SSIDs and passwords, which are stored and remembered by the ESP8266 OS automatically. This entire system assumes that the user has used the WiFi example to connect to their network.
*/
struct eeprom_settings_struct {
    /** Index into the timezone list, defined in zonelist.h */
    uint16_t timezone = 0;
    /** Index into clock face functions list, determines selected clock face. This may be an issue*/
    uint16_t clockFaceIdx = 0;
    bool show_timing = false;
    uint32_t sleepTime = 5 * 60 * 1000;
};

// global settings struct that is declared in the main file
extern eeprom_settings_struct eeprom_settings;

// macro for size of setting data
constexpr size_t SETTINGS_SIZE = sizeof(eeprom_settings_struct);
// macro for address of setting data. Because no other eeprom storage is used, 0 can be used here.
constexpr size_t SETTINGS_ADDRESS = 0;

/** Helper function that stores the settings struct in EEPROM, if it changed. */
void saveSettings();
/** Helper function that creates the default settings in case the settings were not properly stored. */
eeprom_settings_struct createDefaultSettings();
