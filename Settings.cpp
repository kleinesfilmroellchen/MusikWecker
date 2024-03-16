#include "Settings.h"
#include "Debug.h"

void save_settings()
{
	EepromSettings stored_settings;
	EEPROM.get(SETTINGS_ADDRESS, stored_settings);
	auto are_same = memcmp(&stored_settings, &eeprom_settings, SETTINGS_SIZE) == 0;
	if (are_same) {
		debug_print(F("Settings didn't change; not writing EEPROM."));
	} else {
		debug_print(F("Writing settings to EEPROM..."));
		EEPROM.put(SETTINGS_ADDRESS, eeprom_settings);
		if (EEPROM.commit()) {
			debug_print(F(" Writing success."));
		} else {
			debug_print(F(" Writing FAILED."));
		}
	}
}
