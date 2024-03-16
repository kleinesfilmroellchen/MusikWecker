#include "FileMenu.h"
#include "Debug.h"
#include "Globals.h"
#include "PrintString.h"
#include "SettingsMenu.h"
#include "strings.h"
#include <SdFat.h>
#include <memory>
#include <stdint.h>

FileSelectMenu::FileSelectMenu(FileOperation operation)
	: operation(operation)
{
	update_directory();
}

void FileSelectMenu::update_directory()
{
	debug_print(current_directory);

	FsFile directory = card.open(current_directory);
	directory.rewindDirectory();

	menu_entries.clear();
	menu_strings.clear();

	menu_entries.push_back(MenuEntry { current_directory.c_str(), nullptr });
	menu_entries.push_back(MenuEntry { PSTR(".. (Elternverzeichnis)"), nullptr });
	while (true) {
		FsFile entry = directory.openNextFile();
		if (!entry)
			break;
		PrintString name;
		entry.printName(&name);
		menu_strings.push_back(name.getString());
		menu_entries.push_back(MenuEntry { menu_strings.back().c_str(), nullptr });
	}
	current_delegate = std::make_unique<OptionsMenu>(menu_entries);
}

Menu* FileSelectMenu::draw_menu(Display* display, uint16_t delta_millis)
{
	current_delegate->draw_menu(display, delta_millis);
	return this;
}

bool FileSelectMenu::should_refresh(uint16_t delta_millis)
{
	if (current_delegate->parent == nullptr)
		current_delegate->parent = this->parent;

	return true;
}

Menu* FileSelectMenu::handle_button(uint8_t buttons)
{
	if (buttons & B_UP || buttons & B_DOWN) {
		current_delegate->handle_button(buttons);
		return this;
	}

	if (buttons & B_LEFT)
		return this->parent;

	if (buttons & B_RIGHT) {
		if (!isFileSelectingOperation(operation)) {
			return current_delegate->handle_button(buttons);
		}

		uint16_t index = current_delegate->get_current_menu();
		// ".." pseudo entry to move to parent
		if (index == 1) {
			auto lastSlash = current_directory.lastIndexOf('/');
			// we're in the root already
			if (current_directory.length() == 1 && lastSlash >= 0)
				return this;

			auto parent = current_directory.substring(0, lastSlash);
			current_directory = parent;
			if (current_directory.isEmpty())
				current_directory = "/";
			update_directory();
		} else if (index == 0 && operation == FileOperation::SelectMoveTarget) {
			// own directory; perform move
			perform_file_action(current_directory + "/" + source_file.substring(source_file.lastIndexOf('/') + 1));
		} else if (index == 0) {
			update_directory();
		} else {
			index -= 2;
			FsFile directory = card.open(current_directory);
			directory.rewindDirectory();
			FsFile child = directory.openNextFile();
			while (index != 0) {
				child = directory.openNextFile();
				--index;
			}
			PrintString base_name;
			child.printName(&base_name);
			String menu_selected_file;
			if (current_directory.indexOf('/') == current_directory.length() - 1) {
				menu_selected_file = current_directory + base_name.getString();
			} else {
				menu_selected_file = current_directory + "/" + base_name.getString();
			}

			if (child.isDirectory()) {
				current_directory = menu_selected_file;
				update_directory();
			} else {
				perform_file_action(menu_selected_file);
			}
		}

		return this;
	}

	return this;
}

void FileSelectMenu::perform_file_action(String chosen_file)
{
	switch (operation) {
	case FileOperation::None:
		break;
	case FileOperation::Move: {
		operation = FileOperation::SelectMoveTarget;
		source_file = chosen_file;
		auto lastSlash = current_directory.lastIndexOf('/');
		// we're not in the root
		if (current_directory.length() > 1 || lastSlash < 0) {
			auto parent = current_directory.substring(0, lastSlash);
			current_directory = parent;
		}
		update_directory();
		break;
	}
	case FileOperation::Delete:
		operation = FileOperation::ConfirmDelete;
		source_file = chosen_file;
		current_delegate = std::make_unique<SettingsMenu<YesNoSelection>>(
			confirm_delete_label, [this](auto x) { this->perform_delete(x); },
			yesNoOptions, yes_no_menu);
		break;
	case FileOperation::SelectMoveTarget:
		operation = FileOperation::ConfirmMove;
		target_file = chosen_file;
		current_delegate = std::make_unique<SettingsMenu<YesNoSelection>>(
			confirm_move_label, [this](auto x) { this->perform_move(x); },
			yesNoOptions, yes_no_menu);
		break;
	// Should not happen...
	case FileOperation::ConfirmDelete:
	case FileOperation::ConfirmMove:
		current_delegate->handle_button(B_RIGHT);
		break;
	}
}

void FileSelectMenu::perform_delete(YesNoSelection selection)
{
	if (selection == YesNoSelection::Yes) {
		debug_print("deleting file");
		FsFile file_to_delete = card.open(source_file, O_RDWR);
		// TODO: handle delete error
		bool status = file_to_delete.remove();
	}

	operation = FileOperation::Delete;
	update_directory();
}

void FileSelectMenu::perform_move(YesNoSelection selection)
{
	if (selection == YesNoSelection::Yes) {
		FsFile file_to_move = card.open(source_file, O_RDWR);
		// TODO: handle move error
		file_to_move.rename(target_file.c_str());
	}

	operation = FileOperation::Move;
	update_directory();
}
