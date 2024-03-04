/** File menu class header. */

#pragma once

#include "MenuClass.h"
#include "OptionsMenu.h"
#include "SettingsMenu.h"
#include "defs.h"
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

constexpr bool isFileSelectingOperation(FileOperation operation)
{
    return operation == FileOperation::None || operation == FileOperation::Move || operation == FileOperation::SelectMoveTarget || operation == FileOperation::Delete;
}

class FileSelectMenu : public Menu {
public:
    FileSelectMenu(FileOperation);

    Menu* drawMenu(T_DISPLAY* disp, uint16_t deltaMillis) override;
    bool shouldRefresh(uint16_t deltaMillis) override;
    Menu* handleButton(uint8_t buttons) override;

private:
    void updateDirectory();

    void performFileAction(String chosenFile);
    void performDelete(YesNoSelection selection);
    void performMove(YesNoSelection selection);

    std::unique_ptr<OptionsMenu> currentDelegate {};
    std::vector<MenuEntry> menuEntries {};
    // menu entries are non-owning
    std::vector<String> menuStrings {};
    String currentDirectory = "/";
    FileOperation operation;

    String sourceFile = "";
    String targetFile = "";
};
