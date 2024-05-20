#include "OptionsMenu.h"
#include "Audio.h"
#include "Debug.h"
#include "Globals.h"
#include "TimeManager.h"
#include <algorithm>

OptionsMenu::OptionsMenu(Span<MenuEntry> menus)
	: menus(menus)
{
	for (auto& menu : menus) {
		if (menu.menu != nullptr)
			menu.menu->parent = this;
	}
}

void OptionsMenu::perform_menu_draw(Display* display, uint8_t width,
	uint8_t height)
{
	fix_top_menu(height / LINE_HEIGHT);

	if (size() == 0)
		return;

	// Volatile in the entire function prevents compiler optimization that kills
	// logic index of first and last menu text to draw
	uint16_t firstMenu = this->current_top_menu,
			 lastMenu = min(this->current_top_menu + (height / LINE_HEIGHT) - 1,
				 static_cast<int>(size() - 1));

	// Use XOR drawing mode. This will make the selected text inverted without
	// further processing required.
	display->setDrawColor(2);
	display->setFont(MAIN_FONT);
	// loop two variables, i is the menu position, menuIdx is the menu text array
	// index
	for (uint16_t menuIdx = firstMenu, i = 0; menuIdx <= lastMenu;
		 ++i, ++menuIdx) {
		String display_string = this->menus[menuIdx].text;
		// highlight current element
		if (menuIdx == this->current_menu)
			display->drawBox(0, position_of_line(i), width, LINE_HEIGHT);
		// issue draw call to display
		draw_string(display, display_string.c_str(), i);
		yield();
	}
}

Menu* OptionsMenu::draw_menu(Display* display, uint16_t delta_millis)
{
	this->dirty = false;
	display->firstPage();
	do {
		perform_menu_draw(display, display->getDisplayWidth(),
			display->getDisplayHeight());

		yield();
	} while (display->nextPage());
	return this;
}

bool OptionsMenu::should_refresh(uint16_t delta_millis) { return this->dirty; }

void OptionsMenu::fix_top_menu(uint8_t line_count)
{
	auto startTop = this->current_top_menu;
	auto startCurrent = this->current_menu;

	if (size() == 0) {
		this->current_top_menu = 0;
		this->current_menu = 0;
	} else {

		// b/c unsigned bytes, use max value to detect overflow when decrementing
		if (this->current_menu == 0xffff)
			this->current_menu = size() - 1;
		// check current menu bounds. for now, going beyond the last menu entry will
		// wrap back to the start
		if (this->current_menu >= size())
			this->current_menu = 0;

		// current menu moved beyond the last menu on display, move top menu down
		while (this->current_menu > (this->current_top_menu + line_count - 1))
			++this->current_top_menu;
		// current menu moved above the first menu on display, move top menu up
		if (this->current_menu < this->current_top_menu)
			this->current_top_menu = this->current_menu;
	}

	if (startTop != this->current_top_menu || startCurrent != this->current_menu)
		this->dirty = true;
}

Menu* OptionsMenu::handle_button(uint8_t buttons)
{
	if (buttons != 0)
		this->dirty = true;

	// back and enter button logic
	if (buttons & BUTTON_LEFT) {
		debug_print(reinterpret_cast<uintptr_t>(this->parent));
		return this->parent;
	}

	if ((buttons & BUTTON_RIGHT) && size() > 0) {
		return this->menus[this->current_menu].menu.get();
	}

	// down and up button (next/previous element) logic
	if (buttons & BUTTON_DOWN)
		++this->current_menu;
	else if (buttons & BUTTON_UP)
		--this->current_menu;

	fix_top_menu(LINE_COUNT);

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

DelegateOptionsMenu::DelegateOptionsMenu(std::vector<MenuEntry> menus)
	: OptionsMenu(Span<MenuEntry> { menus })
	, fake_entries(std::move(menus))
{
}

Menu* DelegateOptionsMenu::handle_button(uint8_t buttons)
{
	if (buttons & BUTTON_LEFT || buttons & BUTTON_UP || buttons & BUTTON_DOWN)
		return OptionsMenu::handle_button(buttons);
	if (buttons & BUTTON_RIGHT) {
		auto* childResult = option_selected(this->current_menu);
		return childResult == nullptr ? this->parent : childResult;
	}
	return this;
}

ClockFaceSelectMenu::ClockFaceSelectMenu(Span<char const*> face_names,
	Span<ClockFaces::ClockFace> clock_faces)
	: DelegateOptionsMenu(DelegateOptionsMenu::create(face_names))
	, clock_faces(clock_faces)
{
	// adjust current menu point to the clock face in the settings
	if (eeprom_settings.clock_settings.clock_face_index < size()) {
		this->current_menu = eeprom_settings.clock_settings.clock_face_index;
		// current menu moved beyond the last menu on display, move top menu down
		while (this->current_menu > (this->current_top_menu + LINE_COUNT - 1))
			++this->current_top_menu;
		// current menu moved above the first menu on display, move top menu up
		if (this->current_menu < this->current_top_menu)
			this->current_top_menu = this->current_menu;

		// actually change the global variable to match the stored clock face index
		current_clock_face = this->clock_faces[this->current_menu];
	}
}

Menu* ClockFaceSelectMenu::handle_button(uint8_t buttons)
{
	time_since_button = last_update;
	return DelegateOptionsMenu::handle_button(buttons);
}

Menu* ClockFaceSelectMenu::option_selected(uint16_t menu_index)
{
	// change the global variable
	current_clock_face = this->clock_faces[menu_index];

	// save settings in eeprom
	eeprom_settings.clock_settings.clock_face_index = menu_index;
	save_settings();

	return nullptr;
}

bool ClockFaceSelectMenu::should_refresh(uint16_t delta_millis)
{
	last_update += delta_millis;
	return last_update - time_since_button > CLOCK_PREVIEW_DELAY || DelegateOptionsMenu::should_refresh(delta_millis);
}

Menu* ClockFaceSelectMenu::draw_menu(Display* display, uint16_t delta_millis)
{
	if (last_update - time_since_button > CLOCK_PREVIEW_DELAY) {
		time_since_button = last_update - CLOCK_PREVIEW_DELAY - 1;
		auto clockFace = this->clock_faces[this->current_menu];

		ace_time::ZonedDateTime curtime = TimeManager::the().current_time();
		display->firstPage();
		do {
			display->setMaxClipWindow();
			display->setClipWindow(0, 0, display->getDisplayWidth() / 2, display->getDisplayHeight());
			perform_menu_draw(display, display->getDisplayWidth() / 2, display->getDisplayHeight());

			yield();
			display->setDrawColor(1);
			display->setMaxClipWindow();
			display->setClipWindow(display->getDisplayWidth() / 2, 0,
				display->getDisplayWidth(),
				display->getDisplayHeight());
			clockFace(display, &curtime, 0.0, display->getDisplayWidth() / 2, 0,
				display->getDisplayWidth() / 2, display->getDisplayHeight());

			yield();
		} while (display->nextPage());
		display->setMaxClipWindow();
	} else {
		return DelegateOptionsMenu::draw_menu(display, delta_millis);
	}

	return this;
}

TimeZoneSelectMenu::TimeZoneSelectMenu()
	: DelegateOptionsMenu(DelegateOptionsMenu::create(tzlist))
{
	// adjust current menu point to the timezone in the settings
	if (eeprom_settings.timezone < ZONE_COUNT) {
		this->current_menu = eeprom_settings.timezone;

		fix_top_menu(LINE_COUNT);

		TimeManager::the().set_zone(this->current_menu);
	}
}

Menu* TimeZoneSelectMenu::option_selected(uint16_t menu_index)
{
	TimeManager::the().set_zone(menu_index);
	return nullptr;
}
