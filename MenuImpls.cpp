/**
   Small Menu implementation classes code.
*/

#include "MenuImpls.h"
#include "DiagnosticMenu.h"
#include "SettingsMenu.h"
#include <limits>
#include <memory>

extern NTPClient timeClient;

Menu* NothingMenu::drawMenu(T_DISPLAY* display, uint16_t deltaMillis)
{
    return this->parent;
}
bool NothingMenu::shouldRefresh(uint16_t _) { return true; }
Menu* NothingMenu::handleButton(uint8_t _) { return this->parent; }

void applyDebugSettings(YesNoSelection yesNo)
{
    eeprom_settings.show_timing = yesNo == YesNoSelection::Yes;
    saveSettings();
}

enum class AutoDisable {
    No,
    Secs10,
    Mins5,
    Mins10,
    Mins30,
};

static std::array<AutoDisable, 5> autoDisableOptions = {
    AutoDisable::No, AutoDisable::Secs10, AutoDisable::Mins5,
    AutoDisable::Mins10, AutoDisable::Mins30
};

void applyAutoDisableSettings(AutoDisable autoDisable)
{
    switch (autoDisable) {
    case AutoDisable::No:
        eeprom_settings.sleepTime = std::numeric_limits<uint32_t>::max();
        break;
    case AutoDisable::Secs10:
        eeprom_settings.sleepTime = 10 * 1000;
        break;
    case AutoDisable::Mins5:
        eeprom_settings.sleepTime = 5 * 60 * 1000;
        break;
    case AutoDisable::Mins10:
        eeprom_settings.sleepTime = 10 * 60 * 1000;
        break;
    case AutoDisable::Mins30:
        eeprom_settings.sleepTime = 30 * 60 * 1000;
        break;
    }
    saveSettings();
}

Menu* createMenuStructure(ace_time::TimeZone* mainTZ)
{
    static std::array<ClockFace, 6> allClockFaces {
        &basicDigitalCF,
        &digitalWithSecondsCF,
        &basicAnalogCF,
        &rotatingSegmentAnalogCF,
        &binaryCF,
        &fullDayBinaryCF,
    };
    // initialize array
    auto cfMenu = std::make_unique<ClockFaceSelectMenu>(design_menu, allClockFaces);

    static std::array<MenuEntry, 3> fileSubmenus = {
        MenuEntry { file_menu_0,
            std::make_unique<FileSelectMenu>(FileOperation::None) },
        MenuEntry { file_menu_1,
            std::make_unique<FileSelectMenu>(FileOperation::Move) },
        MenuEntry { file_menu_2,
            std::make_unique<FileSelectMenu>(FileOperation::Delete) },
    };
    auto fileMenu = std::make_unique<OptionsMenu>(fileSubmenus);

    auto debuggingMenu = std::make_unique<SettingsMenu<YesNoSelection>>(
        debuggingLabel, &applyDebugSettings, yesNoOptions, yes_no_menu);
    auto autoDisableMenu = std::make_unique<SettingsMenu<AutoDisable>>(
        autoDisableLabel, &applyAutoDisableSettings, autoDisableOptions,
        auto_disable_menu);

    static std::array<MenuEntry, 3> settingsSubmenus = {
        MenuEntry { settings_menu_0, std::move(autoDisableMenu) },
        MenuEntry { settings_menu_1, std::make_unique<TimeZoneSelectMenu>() },
        MenuEntry { settings_menu_2, std::move(debuggingMenu) },
    };
    auto settingsMenu = std::make_unique<OptionsMenu>(settingsSubmenus);

    // TODO: replace temporary nothing menus by the actual menus once implemented
    static std::array<MenuEntry, 5> allMenus {
        MenuEntry { main_menu_0, std::move(cfMenu) },
        MenuEntry { main_menu_1, std::make_unique<NothingMenu>() },
        MenuEntry { main_menu_2, std::move(fileMenu) },
        MenuEntry { main_menu_3, std::move(settingsMenu) },
        MenuEntry { main_menu_4, std::make_unique<DiagnosticMenu>() },
    };
    OptionsMenu* mainMenu = new OptionsMenu(allMenus);

    ClockMenu* clk = new ClockMenu(&timeClient, mainTZ, (Menu*)mainMenu);

    return clk;
}
