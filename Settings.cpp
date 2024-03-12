#include "Settings.h"
#include "Debug.h"

void saveSettings()
{
    eeprom_settings_struct storedSettings;
    EEPROM.get(SETTINGS_ADDRESS, storedSettings);
    uint8_t *storedPtr = (uint8_t*)&storedSettings,
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

eeprom_settings_struct createDefaultSettings()
{
    return eeprom_settings_struct();
}
