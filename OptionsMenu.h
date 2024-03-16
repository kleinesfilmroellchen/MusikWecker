#pragma once

#include "ClockFaces.h"
#include "Definitions.h"
#include "DisplayUtils.h"
#include "Menu.h"
#include "Settings.h"
#include "Span.h"
#include "ace_time/LocalDateTime.h"
#include "zonelist.h"
#include <AceTime.h>
#include <EEPROM.h>
#include <U8g2lib.h>

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
	/** Flag that specifies whether the menu display structure changed. Set by handle_button() and reset by draw_menu(). */
	bool dirty = true;

	/** Implementation of menu drawing that subclasses can use to draw the screen anywhere */
	void perform_menu_draw(Display* display, uint8_t width, uint8_t height);

	void fix_top_menu(uint8_t line_count);

public:
	OptionsMenu(Span<MenuEntry> menues);
	Menu* draw_menu(Display* display, uint16_t delta_millis) override;
	bool should_refresh(uint16_t delta_millis) override;
	Menu* handle_button(uint8_t buttons) override;

	constexpr bool is_dirty() const { return dirty; }
	constexpr uint16_t get_current_menu() const { return currentMenu; }

	constexpr size_t size() const { return menues.size(); }
};

/**
	Options menu that doesn't have any real entries; whenever an entry is selected the child handles it.
*/
class DelegateOptionsMenu : public OptionsMenu {
public:
	static DelegateOptionsMenu create(Span<char const*> names);

	Menu* handle_button(uint8_t buttons) override;

	/**
	 Gets called whenever one of the options is selected.
	Children can override this to run their own handler when this happens, and return any new menu they like.
	If nullptr is returned, the parent menu is returned instead as a default action.
	*/
	virtual Menu* option_selected(uint16_t menuIndex) { return nullptr; }

private:
	DelegateOptionsMenu(std::vector<MenuEntry> menues);

	std::vector<MenuEntry> fakeEntries;
};

/**
   A special options menu that allows for the selection of the currently active clock face. It will use its text entries to select one of the clock faces given by the constructor function pointer array.
*/
class ClockFaceSelectMenu : public DelegateOptionsMenu {
public:
	ClockFaceSelectMenu(Span<char const*> face_names, Span<ClockFaces::ClockFace> clock_faces);
	Menu* draw_menu(Display* display, uint16_t delta_millis) override;
	bool should_refresh(uint16_t delta_millis) override;
	Menu* handle_button(uint8_t buttons) override;

	virtual Menu* option_selected(uint16_t menuIndex) override;

private:
	Span<ClockFaces::ClockFace> clock_faces;
	uint16_t time_since_button { 0 };
	uint16_t last_update { 0 };
};

/**
 * A special options menu that allows for the selection of the active timezone.
 */
class TimeZoneSelectMenu : public DelegateOptionsMenu {
public:
	TimeZoneSelectMenu();
	virtual Menu* option_selected(uint16_t menuIndex) override;
};
