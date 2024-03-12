#include "FileMenu.h"
#include "Debug.h"
#include "PrintString.h"
#include "SettingsMenu.h"
#include "strings.h"
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

    menuEntries.push_back(MenuEntry { currentDirectory.c_str(), nullptr });
    menuEntries.push_back(MenuEntry { PSTR(".. (Elternverzeichnis)"), nullptr });
    while (true) {
        FsFile entry = directory.openNextFile();
        if (!entry)
            break;
        PrintString name;
        entry.printName(&name);
        menuStrings.push_back(name.getString());
        menuEntries.push_back(MenuEntry { menuStrings.back().c_str(), nullptr });
    }
    currentDelegate = std::make_unique<OptionsMenu>(menuEntries);
}

Menu* FileSelectMenu::drawMenu(T_DISPLAY* disp, uint16_t deltaMillis)
{
    currentDelegate->drawMenu(disp, deltaMillis);
    return this;
}

bool FileSelectMenu::shouldRefresh(uint16_t deltaMillis)
{
    if (currentDelegate->parent == nullptr)
        currentDelegate->parent = this->parent;

    return true;
}

Menu* FileSelectMenu::handleButton(uint8_t buttons)
{
    if (buttons & B_UP || buttons & B_DOWN) {
        currentDelegate->handleButton(buttons);
        return this;
    }

    if (buttons & B_LEFT)
        return this->parent;

    if (buttons & B_RIGHT) {
        if (!isFileSelectingOperation(operation)) {
            return currentDelegate->handleButton(buttons);
        }

        uint16_t index = currentDelegate->getCurrentMenu();
        // ".." pseudo entry to move to parent
        if (index == 1) {
            auto lastSlash = currentDirectory.lastIndexOf('/');
            // we're in the root already
            if (currentDirectory.length() == 1 && lastSlash >= 0)
                return this;

            auto parent = currentDirectory.substring(0, lastSlash);
            currentDirectory = parent;
            if (currentDirectory.isEmpty())
                currentDirectory = "/";
            updateDirectory();
        } else if (index == 0 && operation == FileOperation::SelectMoveTarget) {
            // own directory; perform move
            performFileAction(currentDirectory + "/" + sourceFile.substring(sourceFile.lastIndexOf('/') + 1));
        } else {
            index -= 2;
            FsFile directory = card.open(currentDirectory);
            directory.rewindDirectory();
            FsFile child = directory.openNextFile();
            while (index != 0) {
                child = directory.openNextFile();
                --index;
            }
            PrintString baseName;
            child.printName(&baseName);
            String menuSelectedFile;
            if (currentDirectory.indexOf('/') == currentDirectory.length() - 1) {
                menuSelectedFile = currentDirectory + baseName.getString();
            } else {
                menuSelectedFile = currentDirectory + "/" + baseName.getString();
            }

            if (child.isDirectory()) {
                currentDirectory = menuSelectedFile;
                updateDirectory();
            } else {
                performFileAction(menuSelectedFile);
            }
        }

        return this;
    }

    return this;
}

void FileSelectMenu::performFileAction(String chosenFile)
{
    switch (operation) {
    case FileOperation::None:
        break;
    case FileOperation::Move: {
        operation = FileOperation::SelectMoveTarget;
        sourceFile = chosenFile;
        auto lastSlash = currentDirectory.lastIndexOf('/');
        // we're not in the root
        if (currentDirectory.length() > 1 || lastSlash < 0) {
            auto parent = currentDirectory.substring(0, lastSlash);
            currentDirectory = parent;
        }
        updateDirectory();
        break;
    }
    case FileOperation::Delete:
        operation = FileOperation::ConfirmDelete;
        sourceFile = chosenFile;
        currentDelegate = std::make_unique<SettingsMenu<YesNoSelection>>(
            confirmDeleteLabel, [this](auto x) { this->performDelete(x); },
            yesNoOptions, yes_no_menu);
        break;
    case FileOperation::SelectMoveTarget:
        operation = FileOperation::ConfirmMove;
        targetFile = chosenFile;
        currentDelegate = std::make_unique<SettingsMenu<YesNoSelection>>(
            confirmMoveLabel, [this](auto x) { this->performMove(x); },
            yesNoOptions, yes_no_menu);
        break;
    // Should not happen...
    case FileOperation::ConfirmDelete:
    case FileOperation::ConfirmMove:
        currentDelegate->handleButton(B_RIGHT);
        break;
    }
}

void FileSelectMenu::performDelete(YesNoSelection selection)
{
    if (selection == YesNoSelection::Yes) {
        debug_print("deleting file");
        FsFile fileToDelete = card.open(sourceFile, O_RDWR);
        // TODO: handle delete error
        bool status = fileToDelete.remove();
    }

    operation = FileOperation::Delete;
    updateDirectory();
}

void FileSelectMenu::performMove(YesNoSelection selection)
{
    if (selection == YesNoSelection::Yes) {
        FsFile fileToMove = card.open(sourceFile, O_RDWR);
        // TODO: handle move error
        fileToMove.rename(targetFile.c_str());
    }

    operation = FileOperation::Move;
    updateDirectory();
}
