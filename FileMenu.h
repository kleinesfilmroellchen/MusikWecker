/** File menu class header. */

#pragma once

#include "MenuClass.h"
#include "OptionsMenu.h"
#include "defs.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <vector>

enum class FileOperation {
    None,
    Move,
    Delete,
};

class FileSelectMenu : public Menu {
public:
    FileSelectMenu(FileOperation);

    Menu* drawMenu(T_DISPLAY* disp, uint16_t deltaMillis) override;
    bool shouldRefresh(uint16_t deltaMillis) override;
    Menu* handleButton(uint8_t buttons) override;

private:
    void updateDirectory();

    void performFileAction(String chosenFile);

    OptionsMenu currentDelegate { {} };
    std::vector<MenuEntry> menuEntries {};
    // menu entries are non-owning
    std::vector<String> menuStrings {};
    String currentDirectory = "/";
    FileOperation operation;

    String firstSelectedFile = "";
};
