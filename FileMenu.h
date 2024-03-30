/** File menu class header. */

#pragma once

#include "Definitions.h"
#include "Menu.h"
#include "OptionsMenu.h"
#include "SettingsMenu.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <vector>

enum class FileMenuState : uint8_t {
	// No action performed.
	None,
	// Move file.
	Move,
	// Delete a file.
	Delete,
	// Selecting a target folder for moving.
	SelectMoveTarget,
	// Confirm deletion.
	ConfirmDelete,
	// Confirm moving.
	ConfirmMove,
	MoveError,
	DeleteError,
};

constexpr bool is_file_selecting_operation(FileMenuState operation)
{
	return operation == FileMenuState::None || operation == FileMenuState::Move || operation == FileMenuState::SelectMoveTarget || operation == FileMenuState::Delete;
}

class FileSelectMenu : public Menu {
public:
	FileSelectMenu(FileMenuState);

	Menu* ICACHE_FLASH_ATTR draw_menu(Display* display, uint16_t delta_millis) override;
	bool ICACHE_FLASH_ATTR should_refresh(uint16_t delta_millis) override;
	Menu* ICACHE_FLASH_ATTR handle_button(uint8_t buttons) override;

private:
	void ICACHE_FLASH_ATTR update_directory();

	void ICACHE_FLASH_ATTR perform_file_action(String chosen_file);
	void ICACHE_FLASH_ATTR perform_delete(YesNoSelection selection);
	void ICACHE_FLASH_ATTR perform_move(YesNoSelection selection);

	std::unique_ptr<OptionsMenu> current_delegate {};
	std::vector<MenuEntry> menu_entries {};
	// menu entries are non-owning
	std::vector<String> menu_strings {};
	String current_directory = "/";
	FileMenuState operation;

	String source_file = "";
	String target_file = "";
};
