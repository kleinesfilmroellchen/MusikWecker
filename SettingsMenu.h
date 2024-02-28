
#pragma once

#include "OptionsMenu.h"

template <typename SelectionType>
class SettingsMenu : public DelegateOptionsMenu {
public:
    using Action = void (*)(SelectionType);

    SettingsMenu(char const* labelText, Action action, Span<SelectionType> options, Span<const char*> optionLabels);
    Menu* drawMenu(T_DISPLAY* disp, uint16_t deltaMillis) override;

    virtual Menu* optionSelected(uint16_t menuIndex) override;

private:
    Action action;
    char const* labelText;
    Span<SelectionType> options;
};

template <typename SelectionType>
SettingsMenu<SelectionType>::SettingsMenu(char const* labelText, typename SettingsMenu<SelectionType>::Action action, Span<SelectionType> options, Span<const char*> optionLabels)
    : DelegateOptionsMenu(DelegateOptionsMenu::create(optionLabels))
    , action(action)
    , labelText(labelText)
    , options(options)
{
}

template <typename SelectionType>
Menu* SettingsMenu<SelectionType>::drawMenu(T_DISPLAY* display, uint16_t deltaMillis)
{
    display->firstPage();
    do {
        performMenuDraw(display, display->getDisplayWidth(), display->getDisplayHeight() - LINE_HEIGHT * 2);
        String label = FPSTR(labelText);
        display->setDrawColor(1);
        display->setFont(MAIN_FONT);
        display->drawHLine(0, linepos(LINE_COUNT - 2) - 1, display->getDisplayWidth());
        drawString(display, label.c_str(), LINE_COUNT - 2);
    } while (display->nextPage());
    display->setMaxClipWindow();

    return this;
}

template <typename SelectionType>
Menu* SettingsMenu<SelectionType>::optionSelected(uint16_t menuIndex)
{
    auto correspondingSelection = options[menuIndex];
    action(correspondingSelection);
    return nullptr;
}

enum class YesNoSelection : bool {
    Yes = true,
    No = false,
};

static std::array<YesNoSelection, 2> yesNoOptions = { YesNoSelection::No, YesNoSelection::Yes };
