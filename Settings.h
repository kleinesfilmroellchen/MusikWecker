/** Defines the eeprom_settings struct which declares the setting data structure in (emulated) EEPROM. */
#pragma once

#include "Definitions.h"
#include <Arduino.h>
#include <EEPROM.h>

/**
   The settings store all user customizable data except wifi SSIDs and passwords, which are stored and remembered by the ESP8266 OS automatically. This entire system assumes that the user has used the WiFi example to connect to their network.
*/
struct EepromSettings {
	/** Index into the timezone list, defined in zonelist.h */
	uint16_t timezone = 0;
	/** Index into clock face functions list, determines selected clock face. This may be an issue*/
	uint16_t clock_face_index = 0;
	bool show_timing = false;
	uint32_t sleep_time = 5 * 60 * 1000;
};

// macro for size of setting data
constexpr size_t SETTINGS_SIZE = sizeof(EepromSettings);
// macro for address of setting data. Because no other eeprom storage is used, 0 can be used here.
constexpr size_t SETTINGS_ADDRESS = 0;

/** Helper function that stores the settings struct in EEPROM, if it changed. */
void save_settings();
