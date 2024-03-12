/**
         Music alarm clock - ESP8266

         by kleinesfilmröllchen

         --------- Hardware:

         SD card reader on hardware SPI (TODO: pinout)

         TODO: pinout for I2S speaker

         OLED screen SSD1306-based I²C on SCL/SDA (TODO: pinout)

         Buttons for menuing
         - Up and Down on analog 0 (voltage divider hack; Up at 3.3V, Down at 0V, Neutral at 1.6V)
         - Left  on pin D3 with internal pull-up
         - Right on pin D0 with external pull-up

         The buttons are not software-debounced, so it is recommended to hardware-debounce them.
         This can simply be done with a 0.1µF (or similar) capacitor in parallel to the button.
*/
//#define FS_NO_GLOBALS
#include "defs.h"
#include "gpio.h"

// libraries
#include <AceTime.h> // advanced time library modeled after java.time with timezone database and complex conversions
#include <EEPROM.h>
#include <ESP8266WiFi.h> // esp wifi library; isn't used much
#include <NTPClient.h> // NTP remotely queries the time
#include <U8g2lib.h> // memory efficient display driver
#include <WiFiUdp.h> // UDP for NTP

extern "C" {
#include <user_interface.h>
}

using namespace ace_time;
#include <spiram-fast.h>

// project includes
#include "Audio.h"
#include "ClockFaces.h"
#include "Debug.h"
#include "DisplayUtils.h"
#include "MenuClass.h"
#include "MenuImpls.h"
#include "Settings.h"
#include "defs.h"
#include "strings.h"
#include "zonelist.h"

void setup();
void loop();

extern eeprom_settings_struct eeprom_settings_default;

//// global constants

// display
T_DISPLAY display(U8G2_R0);
// udp interface
WiFiUDP ntpUDP;
// SD card
SdFs card;
// ntp client: udp system, ntp server, time offset, update interval
// b/c other library does time zone adjustments, no time offset is applied here
NTPClient timeClient(ntpUDP, TEMP_TIME_SERVER, 0, TEMP_NTP_UPDATE_INTERVAL);
CompleteZoneProcessorCache<TZ_CACHE_SIZE> zoneProcessorCache;
// time zone manager
CompleteZoneManager manager(
    zonedbc::kZoneRegistrySize, zonedbc::kZoneRegistry, zoneProcessorCache);
// main timezone
TimeZone* mainTZ;

eeprom_settings_struct eeprom_settings;

//// global vars

// state of the buttons in the previous loop
uint8_t lastButtons = 0xff;
// ms time when the buttons last changed; use this to register repeated button presses on button hold
uint32_t buttonChangeTime = 0;
// ms delta since the last time a button press was executed when holding a button
int32_t buttonHoldTimeDelta = 0;
// ms time when the last loop execution happened
uint32_t previousLoopTime = 0;
// ms time since the last draw call was issued
uint32_t drawTime = 0;
// whether the NTP client has just updated
bool ntpUpdateOccurred = false;

// currently open menu, e.g. clock or settings
Menu* currentMenu = nullptr;
// pointer to current function pointer responsible for clock face drawing
ClockFace curClockFace;

void setup()
{
// comment this if you need the serial pins for i/o
#if USE_SERIAL
    Serial.begin(115200, SerialConfig::SERIAL_8N1, SerialMode::SERIAL_TX_ONLY);
    while (!Serial)
        yield();
    Serial.println();
#endif

    // connect display first for debugging
    if (!display.begin()) {
        // Force initialize serial port to allow for better debugging; we're resetting in a moment anyways
        Serial.begin(115200, SerialConfig::SERIAL_8N1, SerialMode::SERIAL_TX_ONLY);
        while (!Serial)
            yield();
        Serial.println(F("SSD1306 connection FAILED."));
        ESP.reset();
    } else {
        display.setCursor(LINE_HEIGHT, 0);
        display.setBusClock(DISPLAY_CLOCK_SPEED);
        debug_print(F("Display ok."));
    }
    delay(200);

    // ----------------------------------------------------------
    // init
    // wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(STASSID, STAPSK);
    WiFi.setAutoReconnect(true);
    delay(1000);

    // pins
    pinMode(PIN_BUTTON_LEFT, INPUT_PULLUP);
    pinMode(PIN_BUTTON_RIGHT, INPUT_PULLUP);

    // eeprom
    EEPROM.begin(SETTINGS_SIZE);
    EEPROM.get(SETTINGS_ADDRESS, eeprom_settings);
    // The timezone index should only reach values of ~ 300-400 in extreme future situations.
    // Therefore, the uninitialized state of the fake EEPROM which has all bits set can be used
    // to detect uninitialized or erased EEPROM.
    if (eeprom_settings.timezone == 0xFFFF) {
        debug_print(F("EEPROM uninitialized, replacing by default settings."));
        eeprom_settings = createDefaultSettings();
        saveSettings();
    }

    // connect SD card
    // "Note that even if you don’t use the hardware SS pin, it must be left as an output or the SD library won’t work."
    pinMode(SS, OUTPUT);
    bool sdConnectResult = card.begin(SD_CONFIG);
    if (!sdConnectResult) {
        debug_print(F("Connection to SD card FAILED. Error code: "));
        debug_print(card.sdErrorCode());
        debug_print(F(" Data: "));
        debug_print(card.sdErrorData());
        // ESP.reset();
    } else {
        debug_print(F("SD ok."));
    }
    yield();

    debug_print(F("Trying to setup audio..."));
#if USE_SERIAL
    audioLogger = &Serial;
#endif
    audioSetup();

    display.firstPage();
    do {
        display.setFont(MAIN_FONT);
        // text should not override display data behind characters, this enables "transparency"
        display.setFontMode(1);
    } while (display.nextPage());

    // initialize menu
    // malloc time zone
    mainTZ = (TimeZone*)malloc(sizeof(TimeZone));
    // create time zone, will later be dependent on user settings
    *mainTZ = manager.createForZoneName("Europe/Berlin");
    // create menu structure
    currentMenu = createMenuStructure(mainTZ);

    yield();

    // start ntp client
    timeClient.begin();
    if (WiFi.status() == WL_CONNECTED) {
        ntpUpdateOccurred = timeClient.update();
    }
    yield();

    // initialize timing variables
    buttonChangeTime = previousLoopTime = drawTime = millis();
}

void loop()
{
    // the timing millisecond counter that rolls around every minute or so
    auto currentLoopTime = millis();

    audioLoop();

    // sync own time with ntp server, if wifi is available.
    if (WiFi.status() == WL_CONNECTED) {
        ntpUpdateOccurred = timeClient.update();
    } else {
        ntpUpdateOccurred = false;
    }

    audioLoop();

    // read buttons, some bit magic here
    volatile uint8_t buttons = 0x0f & (((analogRead(PIN_BUTTON_UPDOWN) > 750) << B_UP_BIT) | ((analogRead(PIN_BUTTON_UPDOWN) < 350) << B_DOWN_BIT) | ((~digitalRead(PIN_BUTTON_RIGHT) & 1) << B_RIGHT_BIT) | ((~digitalRead(PIN_BUTTON_LEFT) & 1) << B_LEFT_BIT));
    // debug_print(buttons, BIN); Serial.flush();
    audioLoop();

    // if a button is held and the time since button change exceeds the hold time "delay"...
    if ((currentLoopTime - buttonChangeTime > BUTTON_HOLD_DELAY) && (buttons != 0)) {
        // decrease delta to next simulated button press by loop delta
        buttonHoldTimeDelta -= currentLoopTime - previousLoopTime;
    } else {
        // else reset delta
        buttonHoldTimeDelta = BUTTON_HOLD_REPEAT_DELAY;
    }

    audioLoop();

    // temporary storage for a possibly different new menu
    Menu* newMenu = currentMenu;
    // handle buttons
    if (buttons != lastButtons || (buttonHoldTimeDelta < 0)) {
        newMenu = newMenu->handleButton(buttons);
    }
    audioLoop();
    // any button state is different: update last button time
    if (buttons != lastButtons) {
        buttonChangeTime = currentLoopTime;
        buttonHoldTimeDelta = BUTTON_HOLD_REPEAT_DELAY;
    }
    // reset time to next simulated button press if a button press was just simulated
    if (buttonHoldTimeDelta < 0) {
        buttonHoldTimeDelta += BUTTON_HOLD_REPEAT_DELAY;
    }

    yield();
    audioLoop();

    // draw if menu changed due to buttons
    if (currentMenu != newMenu) {
        newMenu = newMenu->drawMenu(&display, currentLoopTime - drawTime);
    }
    // draw if menu wants to refresh
    else if (newMenu->shouldRefresh(currentLoopTime - previousLoopTime)) {
        audioLoop();
        currentMenu = newMenu->drawMenu(&display, currentLoopTime - drawTime);
        drawTime = currentLoopTime;
    }

    audioLoop();

    // store new menu
    currentMenu = newMenu;

    // TODO: light sleep regularly crashes the ESP8266, and I don't quite know why.
    // The entire light sleep setup itself is rather finnicky in the first place, and there's zero good documentation on it.
    // Since the clock can restore its state with ease after a power cycle (usually needing <10s to reconnect to Wifi and fetching NTP),
    // this is not really an issue, but it annoyingly makes the screen turn on sporadically.
    if (currentLoopTime - buttonChangeTime > eeprom_settings.sleepTime) {
        debug_print(F("Running light sleep..."));
        display.setPowerSave(true);

        delay(10);
        wifi_set_opmode(NULL_MODE);
        wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
        wifi_fpm_open();
        // allow both of these to interrupt sleep
        gpio_pin_wakeup_enable(GPIO_ID_PIN(PIN_BUTTON_LEFT), GPIO_PIN_INTR_LOLEVEL);
        // doesn't seem to work...
        gpio_pin_wakeup_enable(GPIO_ID_PIN(PIN_BUTTON_RIGHT), GPIO_PIN_INTR_LOLEVEL);
        // TODO: Use time until next alarm for the maximum sleep time
        wifi_fpm_do_sleep(0xFFFFFFFF);
        delay(10);

        // disable power save again
        display.setPowerSave(false);
        wifi_fpm_set_sleep_type(NONE_SLEEP_T);
        WiFi.forceSleepWake();
        WiFi.begin();
        timeClient.forceUpdate();
        buttonChangeTime = currentLoopTime = millis();
    }
    yield();
    audioLoop();

    // store state and time
    lastButtons = buttons;
    previousLoopTime = currentLoopTime;
}
