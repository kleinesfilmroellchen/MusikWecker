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
#include "strings.h"
#include <limits>
#include <memory>

Menu* NothingMenu::draw_menu(Display* display, uint16_t delta_millis)
{
	return this->parent;
}
bool NothingMenu::should_refresh(uint16_t _) { return true; }
Menu* NothingMenu::handle_button(uint8_t _) { return this->parent; }

void apply_debug_settings(YesNoSelection yes_no)
{
	eeprom_settings.show_timing = yes_no == YesNoSelection::Yes;
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

Menu* create_menu_structure(ace_time::TimeZone* main_time_zone)
{
	using namespace ClockFaces;
	static std::array<ClockFace, 6> clock_faces {
		&basic_digital,
		&digital_with_seconds,
		&basic_analog,
		&rotating_segment_analog,
		&binary,
		&day_seconds_binary,
	};
	auto clock_face_menu = std::make_unique<ClockFaceSelectMenu>(design_menu, clock_faces);

	static std::array<MenuEntry, 3> file_submenus = {
		MenuEntry { file_menu_0,
			std::make_unique<FileSelectMenu>(FileOperation::None) },
		MenuEntry { file_menu_1,
			std::make_unique<FileSelectMenu>(FileOperation::Move) },
		MenuEntry { file_menu_2,
			std::make_unique<FileSelectMenu>(FileOperation::Delete) },
	};
	auto file_menu_object = std::make_unique<OptionsMenu>(file_submenus);

	auto debugging_menu = std::make_unique<SettingsMenu<YesNoSelection>>(
		debugging_label, &apply_debug_settings, yes_no_options, yes_no_menu);
	auto auto_disable_settings = std::make_unique<SettingsMenu<AutoDisable>>(
		auto_disable_label, &apply_auto_disable_settings, auto_disable_options,
		auto_disable_menu);

	static std::array<MenuEntry, 3> settings_submenus = {
		MenuEntry { settings_menu_0, std::move(auto_disable_settings) },
		MenuEntry { settings_menu_1, std::make_unique<TimeZoneSelectMenu>() },
		MenuEntry { settings_menu_2, std::move(debugging_menu) },
	};
	auto settings_menu_object = std::make_unique<OptionsMenu>(settings_submenus);

	// TODO: replace temporary nothing menus by the actual menus once implemented
	static std::array<MenuEntry, 5> all_menus {
		MenuEntry { main_menu_0, std::move(clock_face_menu) },
		MenuEntry { main_menu_1, std::make_unique<NothingMenu>() },
		MenuEntry { main_menu_2, std::move(file_menu_object) },
		MenuEntry { main_menu_3, std::move(settings_menu_object) },
		MenuEntry { main_menu_4, std::make_unique<DiagnosticMenu>() },
	};
	OptionsMenu* main_menu = new OptionsMenu(all_menus);

	ClockMenu* clock = new ClockMenu(&time_client, main_time_zone, static_cast<Menu*>(main_menu));

	return clock;
}
