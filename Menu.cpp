/**
   Small Menu implementation classes code.
*/

#include "Menu.h"
#include "ClockMenu.h"
#include "DiagnosticMenu.h"
#include "FileMenu.h"
#include "Globals.h"
#include "Settings.h"
#include "SettingsMenu.h"
#include "TimeFormatMenu.h"
#include "VideoPlayer.h"
#include "strings.h"
#include <limits>
#include <memory>
#include <umm_malloc/umm_heap_select.h>

Menu* NothingMenu::draw_menu(Display* display, uint16_t delta_millis)
{
	return this->parent;
}
bool NothingMenu::should_refresh(uint16_t _) { return true; }
Menu* NothingMenu::handle_button(uint8_t _) { return this->parent; }

void apply_debug_settings(YesNoSelection yes_no)
{
	eeprom_settings.show_debug = yes_no == YesNoSelection::Yes;
	save_settings();
}

enum class AutoDisable {
	No,
	Secs10,
	Mins5,
	Mins10,
	Mins30,
};

static std::array<AutoDisable, 5> auto_disable_options = {
	AutoDisable::No, AutoDisable::Secs10, AutoDisable::Mins5,
	AutoDisable::Mins10, AutoDisable::Mins30
};

void apply_auto_disable_settings(AutoDisable auto_disable)
{
	switch (auto_disable) {
	case AutoDisable::No:
		eeprom_settings.sleep_time = std::numeric_limits<uint32_t>::max();
		break;
	case AutoDisable::Secs10:
		eeprom_settings.sleep_time = 10 * 1000;
		break;
	case AutoDisable::Mins5:
		eeprom_settings.sleep_time = 5 * 60 * 1000;
		break;
	case AutoDisable::Mins10:
		eeprom_settings.sleep_time = 10 * 60 * 1000;
		break;
	case AutoDisable::Mins30:
		eeprom_settings.sleep_time = 30 * 60 * 1000;
		break;
	}
	save_settings();
}

static std::array<DateFormat, 4> date_settings_options = {
	DateFormat::None,
	DateFormat::ISO8601,
	DateFormat::German,
	DateFormat::GermanShort,
};

void apply_date_settings(DateFormat date_format)
{
	eeprom_settings.clock_settings.date_format = date_format;
	save_settings();
}

Menu* create_menu_structure()
{
	HeapSelectIram iram;

	static std::array<MenuEntry, 3> file_submenus = {
		MenuEntry { file_menu_view,
			std::make_unique<FileSelectMenu>(FileMenuState::None) },
		MenuEntry { file_menu_move,
			std::make_unique<FileSelectMenu>(FileMenuState::Move) },
		MenuEntry { file_menu_delete,
			std::make_unique<FileSelectMenu>(FileMenuState::Delete) },
	};
	static auto file_menu_object = std::make_unique<OptionsMenu>(file_submenus);

	static auto debugging_menu = std::make_unique<SettingsMenu<YesNoSelection>>(
		debugging_label, &apply_debug_settings, yes_no_options, yes_no_menu);
	static auto auto_disable_settings = std::make_unique<SettingsMenu<AutoDisable>>(
		auto_disable_label, &apply_auto_disable_settings, auto_disable_options,
		auto_disable_menu);
	static auto date_settings = std::make_unique<SettingsMenu<DateFormat>>(
		date_settings_label, &apply_date_settings, date_settings_options,
		date_settings_menu);

	static auto clock_face_menu = std::make_unique<ClockFaceSelectMenu>(design_menu, ClockFaces::clock_faces);

	static std::array<MenuEntry, 6> settings_submenus = {
		MenuEntry { settings_menu_auto_disable, std::move(auto_disable_settings) },
		MenuEntry { settings_menu_timezone, std::make_unique<TimeZoneSelectMenu>() },
		MenuEntry { settings_menu_clock_design, std::move(clock_face_menu) },
		MenuEntry { settings_menu_date_format, std::move(date_settings) },
		MenuEntry { settings_menu_time_format, std::make_unique<TimeFormatMenu>() },
		MenuEntry { settings_menu_debugging, std::move(debugging_menu) },
	};
	static auto settings_menu_object = std::make_unique<OptionsMenu>(settings_submenus);

	static std::array<MenuEntry, 5> all_menus {
		MenuEntry { main_menu_alarms, std::make_unique<NothingMenu>() },
		MenuEntry { main_menu_files, std::move(file_menu_object) },
		MenuEntry { main_menu_settings, std::move(settings_menu_object) },
		MenuEntry { main_menu_diagnostics, std::make_unique<DiagnosticMenu>() },
		MenuEntry { main_menu_video, std::make_unique<VideoPlayer>() },
	};
	static OptionsMenu* main_menu = new OptionsMenu(all_menus);

	static ClockMenu* clock = new ClockMenu(static_cast<Menu*>(main_menu));

	return clock;
}
