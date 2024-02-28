#pragma once

#include "MenuClass.h"
#include <stdint.h>

enum class DiagnosticPage : uint8_t {
    Time,
    FileSystem,
    __Count,
};

class DiagnosticMenu : public Menu {
public:
    virtual Menu* drawMenu(T_DISPLAY* disp, uint16_t deltaMillis) override;
    virtual bool shouldRefresh(uint16_t deltaMillis) override;
    virtual Menu* handleButton(uint8_t buttons) override;

private:
    DiagnosticPage currentPage = DiagnosticPage::Time;
    bool dirty = true;
    uint32_t timeOfLastNTP = 0;
};
