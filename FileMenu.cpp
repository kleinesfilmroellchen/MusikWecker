#include "FileMenu.h"
#include "Debug.h"
#include "PrintString.h"
#include <SdFat.h>
#include <memory>
#include <stdint.h>

extern SdFs card;

FileSelectMenu::FileSelectMenu(FileOperation operation)
    : operation(operation)
{
    updateDirectory();
}

void FileSelectMenu::updateDirectory()
{
    debug_print(currentDirectory);
    FsFile directory = card.open(currentDirectory);
    directory.rewindDirectory();

    menuEntries.clear();
    menuStrings.clear();

    menuEntries.push_back(MenuEntry { ".. (Elternverzeichnis)", nullptr });
    while (true) {
        FsFile entry = directory.openNextFile();
        if (!entry)
            break;
        PrintString name;
        entry.printName(&name);
        menuStrings.push_back(name.getString());
        menuEntries.push_back(MenuEntry { menuStrings.back().c_str(), nullptr });
    }
    currentDelegate = OptionsMenu { menuEntries };
}

Menu* FileSelectMenu::drawMenu(T_DISPLAY* disp, uint16_t deltaMillis)
{
    currentDelegate.drawMenu(disp, deltaMillis);
    return this;
}

bool FileSelectMenu::shouldRefresh(uint16_t deltaMillis)
{
    if (currentDelegate.parent == nullptr)
        currentDelegate.parent = this->parent;

    return currentDelegate.isDirty();
}

Menu* FileSelectMenu::handleButton(uint8_t buttons)
{
    if (buttons & B_UP || buttons & B_DOWN) {
        currentDelegate.handleButton(buttons);
        return this;
    }

    if (buttons & B_LEFT)
        return this->parent;

    if (buttons & B_RIGHT) {
        uint16_t index = currentDelegate.getCurrentMenu();
        // ".." pseudo entry to move to parent
        if (index == 0) {
            auto lastSlash = currentDirectory.lastIndexOf('/');
            // we're in the root already
            if (lastSlash <= 0)
                return this;

            auto parent = currentDirectory.substring(0, lastSlash);
            currentDirectory = parent;
        } else {
            --index;
            FsFile directory = card.open(currentDirectory);
            directory.rewindDirectory();
            FsFile child = directory.openNextFile();
            while (index != 0) {
                child = directory.openNextFile();
                --index;
            }
            PrintString baseName;
            child.printName(&baseName);
            if (child.isDirectory()) {
                currentDirectory = currentDirectory + "/" + baseName.getString();
            } else {
                // performFileAction(currentDirectory + "/" + baseName.getString());
            }
        }

        updateDirectory();
        return this;
    }

    return this;
}