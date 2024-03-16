
#pragma once

#include "OptionsMenu.h"
#include <functional>

template <typename SelectionType>
class SettingsMenu : public DelegateOptionsMenu {
public:
	using Action = std::function<void(SelectionType)>;

	SettingsMenu(char const* label_text, Action action, Span<SelectionType> options, Span<const char*> option_labels);
	Menu* draw_menu(Display* display, uint16_t delta_millis) override;

	virtual Menu* option_selected(uint16_t menu_index) override;

private:
	Action action;
	char const* label_text;
	Span<SelectionType> options;
};

template <typename SelectionType>
SettingsMenu<SelectionType>::SettingsMenu(char const* label_text, typename SettingsMenu<SelectionType>::Action action, Span<SelectionType> options, Span<const char*> option_labels)
	: DelegateOptionsMenu(DelegateOptionsMenu::create(option_labels))
	, action(action)
	, label_text(label_text)
	, options(options)
{
}

template <typename SelectionType>
Menu* SettingsMenu<SelectionType>::draw_menu(Display* display, uint16_t delta_millis)
{
	display->firstPage();
	do {
		perform_menu_draw(display, display->getDisplayWidth(), display->getDisplayHeight() - LINE_HEIGHT * 2);
		String label = FPSTR(label_text);
		display->setDrawColor(1);
		display->setFont(MAIN_FONT);
		display->drawHLine(0, position_of_line(LINE_COUNT - 2) - 1, display->getDisplayWidth());
		draw_string(display, label.c_str(), LINE_COUNT - 2);

		yield();
	} while (display->nextPage());
	display->setMaxClipWindow();

	return this;
}

template <typename SelectionType>
Menu* SettingsMenu<SelectionType>::option_selected(uint16_t menu_index)
{
	auto corresponding_selection = options[menu_index];
	action(corresponding_selection);
	return nullptr;
}

enum class YesNoSelection : bool {
	Yes = true,
	No = false,
};

static std::array<YesNoSelection, 2> yes_no_options = { YesNoSelection::No, YesNoSelection::Yes };
