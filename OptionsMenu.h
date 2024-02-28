#pragma once

#include "ClockFaces.h"
#include "DisplayUtils.h"
#include "MenuClass.h"
#include "Settings.h"
#include "Span.h"
#include "ace_time/LocalDateTime.h"
#include "defs.h"
#include "zonelist.h"
#include <AceTime.h>
#include <EEPROM.h>
#include <U8g2lib.h>

extern ClockFace curClockFace;
extern ace_time::TimeZone* mainTZ;
extern eeprom_settings_struct eeprom_settings;

struct MenuEntry {
    __FlashStringHelper const* text;
    // Actual menu to which this entry can navigate.
    std::unique_ptr<Menu> menu;

    template <typename MenuDerived>
    MenuEntry(char const* text, std::unique_ptr<MenuDerived>&& menu)
        : text(FPSTR(text))
        , menu(std::unique_ptr<Menu> { std::forward<std::unique_ptr<MenuDerived>>(menu) })
    {
    }
    MenuEntry(char const* text, std::nullptr_t null)
        : text(FPSTR(text))
        , menu(static_cast<std::unique_ptr<Menu>>(null))
    {
    }
};

/**
   Menu that displays a list of text entries corresponding to sub-menues.
   The menu can navigate to these menues and return to its parent menu.
*/
class OptionsMenu : public Menu {
private:
    Span<MenuEntry> menues;

protected:
    /** Index of the currently selected menu. */
    uint16_t currentMenu = 0;
    /** Index of the menu that is currently on top of the section of menues which are displayed. */
    uint16_t currentTopMenu = 0;
    /** Flag that specifies whether the menu display structure changed. Set by handleButton() and reset by drawMenu(). */
    bool dirty = true;

    /** Implementation of menu drawing that subclasses can use to draw the screen anywhere */
    void performMenuDraw(T_DISPLAY* disp, uint8_t width, uint8_t height);

    void fixTopMenu(uint8_t line_count);

public:
    OptionsMenu(Span<MenuEntry> menues);
    Menu* drawMenu(T_DISPLAY* disp, uint16_t deltaMillis) override;
    bool shouldRefresh(uint16_t deltaMillis) override;
    Menu* handleButton(uint8_t buttons) override;

    constexpr bool isDirty() const { return dirty; }
    constexpr uint16_t getCurrentMenu() const { return currentMenu; }

    constexpr size_t size() const { return menues.size(); }
};

/**
    Options menu that doesn't have any real entries; whenever an entry is selected the child handles it.
*/
class DelegateOptionsMenu : public OptionsMenu {
public:
    static DelegateOptionsMenu create(Span<char const*> names);

    Menu* handleButton(uint8_t buttons) override;

    /**
     Gets called whenever one of the options is selected.
    Children can override this to run their own handler when this happens, and return any new menu they like.
    If nullptr is returned, the parent menu is returned instead as a default action.
    */
    virtual Menu* optionSelected(uint16_t menuIndex) { return nullptr; }

private:
    DelegateOptionsMenu(std::vector<MenuEntry> menues);

    std::vector<MenuEntry> fakeEntries;
};

/**
   A special options menu that allows for the selection of the currently active clock face. It will use its text entries to select one of the clock faces given by the constructor function pointer array.
*/
class ClockFaceSelectMenu : public DelegateOptionsMenu {
public:
    ClockFaceSelectMenu(Span<char const*> face_names, Span<ClockFace> clockFaces);
    Menu* drawMenu(T_DISPLAY* disp, uint16_t deltaMillis) override;
    bool shouldRefresh(uint16_t deltaMillis) override;
    Menu* handleButton(uint8_t buttons) override;

    virtual Menu* optionSelected(uint16_t menuIndex) override;

private:
    Span<ClockFace> clockFaces;
    uint16_t timeSinceButton { 0 };
    uint16_t lastUpdate { 0 };
};

/**
 * A special options menu that allows for the selection of the active timezone.
 */
class TimeZoneSelectMenu : public DelegateOptionsMenu {
public:
    TimeZoneSelectMenu();
    virtual Menu* optionSelected(uint16_t menuIndex) override;
};
