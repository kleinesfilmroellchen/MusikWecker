/** File menu class header. */

#pragma once

#include "Definitions.h"
#include "Menu.h"
#include "OptionsMenu.h"
#include "SettingsMenu.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <vector>

enum class FileOperation {
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
};

constexpr bool is_file_selecting_operation(FileOperation operation)
{
	return operation == FileOperation::None || operation == FileOperation::Move || operation == FileOperation::SelectMoveTarget || operation == FileOperation::Delete;
}

class FileSelectMenu : public Menu {
public:
	FileSelectMenu(FileOperation);

	Menu* draw_menu(Display* display, uint16_t delta_millis) override;
	bool should_refresh(uint16_t delta_millis) override;
	Menu* handle_button(uint8_t buttons) override;

private:
	void update_directory();

	void perform_file_action(String chosen_file);
	void perform_delete(YesNoSelection selection);
	void perform_move(YesNoSelection selection);

	std::unique_ptr<OptionsMenu> current_delegate {};
	std::vector<MenuEntry> menu_entries {};
	// menu entries are non-owning
	std::vector<String> menu_strings {};
	String current_directory = "/";
	FileOperation operation;

	String source_file = "";
	String target_file = "";
};
