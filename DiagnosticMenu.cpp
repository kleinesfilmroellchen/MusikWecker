#include "DiagnosticMenu.h"
#include "Audio.h"
#include "DisplayUtils.h"
#include "NTPClient.h"
#include "PrintString.h"
#include "strings.h"
#include <AceTime.h>
#include <SdFat.h>
#include <WString.h>

extern NTPClient timeClient;
extern bool ntpUpdateOccurred;
extern ace_time::TimeZone* mainTZ;
extern ace_time::CompleteZoneManager manager;

DiagnosticPage operator+(DiagnosticPage const& self, uint8_t a)
{
    return static_cast<DiagnosticPage>((static_cast<uint8_t>(self) + a) % static_cast<uint8_t>(DiagnosticPage::__Count));
}

DiagnosticPage operator-(DiagnosticPage const& self, uint8_t a)
{
    return static_cast<DiagnosticPage>((static_cast<uint8_t>(self) - a) % static_cast<uint8_t>(DiagnosticPage::__Count));
}

extern SdFs card;

Menu* DiagnosticMenu::drawMenu(T_DISPLAY* display, uint16_t deltaMillis)
{
    this->dirty = false;
    display->firstPage();
    do {
        // FIXME: use std::format once Arduino can C++20
        switch (this->currentPage) {
        case DiagnosticPage::Time: {
            this->dirty = true;

            PrintString zoneName;
            mainTZ->printTo(zoneName);

            char timeInfo[128];
            snprintf_P(timeInfo, sizeof(timeInfo), PSTR("unix %ds\ntz %s offset %05ds\nserver %s\nlast sync %d (synced %d)"),
                timeClient.getEpochTime(), zoneName.getString().c_str(), mainTZ->getOffsetDateTime(timeClient.getEpochTime()).timeOffset(),
                TEMP_TIME_SERVER, millis() - timeOfLastNTP, timeClient.isTimeSet());
            display->setFont(TINY_FONT);
            drawString(display, timeInfo, 0);
            break;
        }

        case DiagnosticPage::FileSystem: {
            this->dirty = true;

            auto capacityKiB = static_cast<uint64_t>(card.vol()->clusterCount() / 1024) * card.vol()->bytesPerCluster();
            auto freeKiB = static_cast<uint64_t>(card.vol()->freeClusterCount() / 1024) * card.vol()->bytesPerCluster();
            auto sectorSize = card.vol()->bytesPerCluster() / card.vol()->sectorsPerCluster();
            auto clusterSize = card.vol()->bytesPerCluster();
            auto sdNumericType = card.card()->type() % 4;
            String sdTypeName = FPSTR(sd_types_array[sdNumericType]);

            char fsInfo[256] {};
            snprintf_P(fsInfo, sizeof(fsInfo), PSTR("sd type %s\nerror %d payload %d\n%lldKi cap %lldKi free\nFAT%d: %d secsz %d clusz"),
                sdTypeName, card.sdErrorCode(), card.sdErrorData(), capacityKiB, freeKiB, card.fatType(), sectorSize, clusterSize);
            display->setFont(TINY_FONT);
            drawString(display, fsInfo, 0);

            break;
        }
        case DiagnosticPage::__Count: {
            this->currentPage = DiagnosticPage::Time;
            break;
        }
        }
        audioLoop();
    } while (display->nextPage());
    return this;
}

bool DiagnosticMenu::shouldRefresh(uint16_t deltaMillis)
{
    if (ntpUpdateOccurred)
        this->timeOfLastNTP = millis();

    return dirty;
}

Menu* DiagnosticMenu::handleButton(uint8_t buttons)
{
    if (buttons != 0)
        this->dirty = true;

    if (buttons & B_LEFT)
        return this->parent;

    if (buttons & B_DOWN)
        this->currentPage = this->currentPage + 1;
    if (buttons & B_UP)
        this->currentPage = this->currentPage - 1;

    return this;
}
