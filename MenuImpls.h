/**
   Menu implementation classes header.
*/
#pragma once

#include "defs.h"
#include "strings.h"
#include <AceTime.h>
#include <U8g2lib.h> //memory efficient display driver

#include "ClockFaces.h"
#include "ClockMenu.h"
#include "FileMenu.h"
#include "MenuClass.h"
#include "OptionsMenu.h"

/**
 * Menu that displays some simple text. It can return to its parent menu.
 */
class TextInfoMenu : public Menu {
public:
    Menu* drawMenu(T_DISPLAY* disp, uint16_t deltaMillis) override;
    bool shouldRefresh(uint16_t deltaMillis) override;
    Menu* handleButton(uint8_t buttons) override;
};

/**
 * Dummy menu that does nothing. When executing any action, it redirects back to its parent menu.
 */
class NothingMenu : public Menu {
public:
    Menu* drawMenu(T_DISPLAY* disp, uint16_t deltaMillis) override;
    bool shouldRefresh(uint16_t deltaMillis) override;
    Menu* handleButton(uint8_t buttons) override;
};

/** Helper function that mallocs and creates all menus in the correct structure. Returns the clock menu, the most top-level menu.*/
Menu* createMenuStructure(ace_time::TimeZone* tz);
