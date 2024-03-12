#include "OptionsMenu.h"
#include "Audio.h"
#include "Debug.h"
#include "NTPClient.h"
#include <algorithm>

OptionsMenu::OptionsMenu(Span<MenuEntry> menues)
    : menues(menues)
{
    for (auto& menu : menues) {
        if (menu.menu != nullptr)
            menu.menu->parent = this;
    }
}

void OptionsMenu::performMenuDraw(T_DISPLAY* display, uint8_t width,
    uint8_t height)
{
    fixTopMenu(height / LINE_HEIGHT);

    if (size() == 0)
        return;

    // Volatile in the entire function prevents compiler optimization that kills
    // logic index of first and last menu text to draw
    uint16_t firstMenu = this->currentTopMenu,
             lastMenu = min(this->currentTopMenu + (height / LINE_HEIGHT) - 1,
                 static_cast<int>(size() - 1));

    // Use XOR drawing mode. This will make the selected text inverted without
    // further processing required.
    display->setDrawColor(2);
    display->setFont(MAIN_FONT);
    // loop two variables, i is the menu position, menuIdx is the menu text array
    // index
    for (uint16_t menuIdx = firstMenu, i = 0; menuIdx <= lastMenu;
         ++i, ++menuIdx) {
        String display_string = this->menues[menuIdx].text;
        // highlight current element
        if (menuIdx == this->currentMenu)
            display->drawBox(0, linepos(i), width, LINE_HEIGHT);
        // issue draw call to display
        drawString(display, display_string.c_str(), i);
        yield();
    }
}

Menu* OptionsMenu::drawMenu(T_DISPLAY* display, uint16_t deltaMillis)
{
    this->dirty = false;
    display->firstPage();
    do {
        performMenuDraw(display, display->getDisplayWidth(),
            display->getDisplayHeight());

        yield();
    } while (display->nextPage());
    return this;
}

bool OptionsMenu::shouldRefresh(uint16_t deltaMillis) { return this->dirty; }

void OptionsMenu::fixTopMenu(uint8_t line_count)
{
    auto startTop = this->currentTopMenu;
    auto startCurrent = this->currentMenu;

    if (size() == 0) {
        this->currentTopMenu = 0;
        this->currentMenu = 0;
    } else {

        // b/c unsigned bytes, use max value to detect overflow when decrementing
        if (this->currentMenu == 0xffff)
            this->currentMenu = size() - 1;
        // check current menu bounds. for now, going beyond the last menu entry will
        // wrap back to the start
        if (this->currentMenu >= size())
            this->currentMenu = 0;

        // current menu moved beyond the last menu on display, move top menu down
        while (this->currentMenu > (this->currentTopMenu + line_count - 1))
            ++this->currentTopMenu;
        // current menu moved above the first menu on display, move top menu up
        if (this->currentMenu < this->currentTopMenu)
            this->currentTopMenu = this->currentMenu;
    }

    if (startTop != this->currentTopMenu || startCurrent != this->currentMenu)
        this->dirty = true;
}

Menu* OptionsMenu::handleButton(uint8_t buttons)
{
    if (buttons != 0)
        this->dirty = true;

    // back and enter button logic
    if (buttons & B_LEFT) {
        debug_print(reinterpret_cast<uintptr_t>(this->parent));
        return this->parent;
    }

    if ((buttons & B_RIGHT) && size() > 0) {
        return this->menues[this->currentMenu].menu.get();
    }

    // down and up button (next/previous element) logic
    if (buttons & B_DOWN)
        ++this->currentMenu;
    else if (buttons & B_UP)
        --this->currentMenu;

    fixTopMenu(LINE_COUNT);

    // happens only for up and down buttons
    return this;
}

static std::vector<MenuEntry> make_empty_menu(Span<char const*> names)
{
    std::vector<MenuEntry> menu;
    for (auto const* name : names) {
        menu.push_back(MenuEntry { name, nullptr });
    }
    return menu;
}

DelegateOptionsMenu DelegateOptionsMenu::create(Span<char const*> names)
{
    auto entries = make_empty_menu(names);
    return DelegateOptionsMenu { std::move(entries) };
}

DelegateOptionsMenu::DelegateOptionsMenu(std::vector<MenuEntry> menues)
    : OptionsMenu(Span<MenuEntry> { menues })
    , fakeEntries(std::move(menues))
{
}

Menu* DelegateOptionsMenu::handleButton(uint8_t buttons)
{
    if (buttons & B_LEFT || buttons & B_UP || buttons & B_DOWN)
        return OptionsMenu::handleButton(buttons);
    if (buttons & B_RIGHT) {
        auto* childResult = optionSelected(this->currentMenu);
        return childResult == nullptr ? this->parent : childResult;
    }
    return this;
}

ClockFaceSelectMenu::ClockFaceSelectMenu(Span<char const*> face_names,
    Span<ClockFace> clockFaces)
    : DelegateOptionsMenu(DelegateOptionsMenu::create(face_names))
    , clockFaces(clockFaces)
{
    // adjust current menu point to the clock face in the settings
    if (eeprom_settings.clockFaceIdx < size()) {
        this->currentMenu = eeprom_settings.clockFaceIdx;
        // current menu moved beyond the last menu on display, move top menu down
        while (this->currentMenu > (this->currentTopMenu + LINE_COUNT - 1))
            ++this->currentTopMenu;
        // current menu moved above the first menu on display, move top menu up
        if (this->currentMenu < this->currentTopMenu)
            this->currentTopMenu = this->currentMenu;

        // actually change the global variable to match the stored clock face index
        curClockFace = this->clockFaces[this->currentMenu];
    }
}

Menu* ClockFaceSelectMenu::handleButton(uint8_t buttons)
{
    timeSinceButton = lastUpdate;
    return DelegateOptionsMenu::handleButton(buttons);
}

Menu* ClockFaceSelectMenu::optionSelected(uint16_t menuIndex)
{
    // change the global variable
    curClockFace = this->clockFaces[menuIndex];

    // save settings in eeprom
    eeprom_settings.clockFaceIdx = menuIndex;
    saveSettings();

    return nullptr;
}

bool ClockFaceSelectMenu::shouldRefresh(uint16_t deltaMillis)
{
    lastUpdate += deltaMillis;
    return lastUpdate - timeSinceButton > CLOCK_PREVIEW_DELAY || DelegateOptionsMenu::shouldRefresh(deltaMillis);
}

extern NTPClient timeClient;
extern ace_time::TimeZone* mainTZ;

Menu* ClockFaceSelectMenu::drawMenu(T_DISPLAY* display, uint16_t deltaMillis)
{
    if (lastUpdate - timeSinceButton > CLOCK_PREVIEW_DELAY) {
        timeSinceButton = lastUpdate - CLOCK_PREVIEW_DELAY - 1;
        auto clockFace = this->clockFaces[this->currentMenu];

        ace_time::ZonedDateTime curtime = ace_time::ZonedDateTime::forUnixSeconds64(
            timeClient.getEpochTime(), *mainTZ);
        display->firstPage();
        do {
            display->setMaxClipWindow();
            display->setClipWindow(0, 0, display->getDisplayWidth() / 2,
                display->getDisplayHeight());
            performMenuDraw(display, display->getDisplayWidth() / 2,
                display->getDisplayHeight());

            yield();
            display->setDrawColor(1);
            display->setMaxClipWindow();
            display->setClipWindow(display->getDisplayWidth() / 2, 0,
                display->getDisplayWidth(),
                display->getDisplayHeight());
            clockFace(display, &curtime, display->getDisplayWidth() / 2, 0,
                display->getDisplayWidth() / 2, display->getDisplayHeight());

            yield();
        } while (display->nextPage());
        display->setMaxClipWindow();
    } else {
        return DelegateOptionsMenu::drawMenu(display, deltaMillis);
    }

    return this;
}

extern ace_time::CompleteZoneManager manager;

TimeZoneSelectMenu::TimeZoneSelectMenu()
    : DelegateOptionsMenu(DelegateOptionsMenu::create(tzlist))
{
    // adjust current menu point to the timezone in the settings
    if (eeprom_settings.timezone < ZONE_COUNT) {
        this->currentMenu = eeprom_settings.timezone;

        fixTopMenu(LINE_COUNT);

        // actually change the global timezone variable
        String tzname = FPSTR(tzlist[this->currentMenu]);
        debug_print(F("Stored timezone: "));
        debug_print(tzname);
        *mainTZ = manager.createForZoneName(tzname.c_str());
    }
}

Menu* TimeZoneSelectMenu::optionSelected(uint16_t menuIndex)
{
    // change the global variable
    String tzname = FPSTR(tzlist[menuIndex]);
    *mainTZ = manager.createForZoneName(tzname.c_str());

    // save settings in eeprom
    eeprom_settings.timezone = menuIndex;
    saveSettings();

    return nullptr;
}
