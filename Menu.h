/**
   Menu class file header.
*/
#pragma once

#include "Definitions.h"
#include <AceTime.h>
#include <U8g2lib.h>

class Menu {
public:
	Menu* parent = nullptr;
	/** Called when the menu should be drawn. Delta is the time since the last draw call. Returns the menu that is now active. */
	virtual Menu* draw_menu(Display* display, uint16_t delta_millis) = 0;
	/** Returns whether to issue another draw call. Delta is the time since the last refresh check. */
	virtual bool should_refresh(uint16_t delta_millis) = 0;
	/** Called when button state change occurrs. The parameter is the AND of all buttons currently pressed. Returns the menu that is now active. */
	virtual Menu* handle_button(uint8_t buttons) = 0;
};

/**
 * Menu that displays some simple text. It can return to its parent menu.
 */
class TextInfoMenu : public Menu {
public:
	Menu* draw_menu(Display* display, uint16_t delta_millis) override;
	bool should_refresh(uint16_t delta_millis) override;
	Menu* handle_button(uint8_t buttons) override;
};

/**
 * Dummy menu that does nothing. When executing any action, it redirects back to its parent menu.
 */
class NothingMenu : public Menu {
public:
	Menu* draw_menu(Display* display, uint16_t delta_millis) override;
	bool should_refresh(uint16_t delta_millis) override;
	Menu* handle_button(uint8_t buttons) override;
};

/** Helper function that mallocs and creates all menus in the correct structure. Returns the clock menu, the most top-level menu.*/
Menu* create_menu_structure(ace_time::TimeZone* tz);
