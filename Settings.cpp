#include "Settings.h"
#include "Debug.h"

void save_settings()
{
	EepromSettings stored_settings;
	EEPROM.get(SETTINGS_ADDRESS, stored_settings);
	uint8_t *storedPtr = (uint8_t*)&stored_settings,
			*currentPtr = (uint8_t*)&eeprom_settings;
	for (uint16_t i = 0; i < SETTINGS_SIZE; ++i)
		if (storedPtr[i] != currentPtr[i])
			// imma commit a crime
			goto GOTO_saveSettingsExecute;
	// only hit if the goto never executed; i.e. all setting elements are
	// identical and nothing needs to be stored
	debug_print(F("Settings didn't change; not writing EEPROM."));
	return;

GOTO_saveSettingsExecute:
	debug_print(F("Writing settings to EEPROM..."));
	EEPROM.put(SETTINGS_ADDRESS, eeprom_settings);
	if (EEPROM.commit()) {
		debug_print(F(" Writing success."));
	} else {
		debug_print(F(" Writing FAILED."));
	}
}
