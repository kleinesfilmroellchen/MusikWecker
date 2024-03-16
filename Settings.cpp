#include "Settings.h"
#include "Debug.h"

void save_settings()
{
	debug_print(F("Writing settings to EEPROM..."));
	EEPROM.put(SETTINGS_ADDRESS, eeprom_settings);
	if (EEPROM.commit()) {
		debug_print(F(" Writing success."));
	} else {
		debug_print(F(" Writing FAILED."));
	}
}
