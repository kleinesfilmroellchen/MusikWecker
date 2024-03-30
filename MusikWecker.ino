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

// Needs to be included first in order to set up certain defines for libraries.
#include "Definitions.h"

#include "Audio.h"
#include "ClockFaces.h"
#include "Debug.h"
#include "DisplayUtils.h"
#include "Menu.h"
#include "Settings.h"
#include "TimeManager.h"
#include "strings.h"
#include "zonelist.h"
#include <AceTime.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <U8g2lib.h>
#include <gpio.h>
#include <spiram-fast.h>
#include <umm_malloc/umm_heap_select.h>
#include <user_interface.h>

void setup();
void loop();

//// global constants

// display
Display display(U8G2_R0);
// SD card
SdFs card;

EepromSettings eeprom_settings;

//// global vars

// state of the buttons in the previous loop
uint8_t last_buttons = 0xff;
// ms time when the buttons last changed; use this to register repeated button presses on button hold
uint32_t button_change_time = 0;
// ms delta since the last time a button press was executed when holding a button
int32_t button_hold_time_delta = 0;
// ms time when the last loop execution happened
uint32_t previous_loop_time = 0;
// ms time since the last draw call was issued
uint32_t drawTime = 0;

// currently open menu, e.g. clock or settings
Menu* current_menu = nullptr;
// current function responsible for clock face drawing
ClockFaces::ClockFace current_clock_face;

void setup()
{
#if USE_SERIAL
	Serial.begin(115200, SerialConfig::SERIAL_8N1, SerialMode::SERIAL_TX_ONLY);
	while (!Serial)
		yield();
	Serial.println();
#endif

	{
		HeapSelectIram iram;
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
		yield();
	}

	// ----------------------------------------------------------
	// init
	// wifi
	WiFi.mode(WIFI_STA);
	WiFi.begin(STASSID, STAPSK);
	WiFi.setAutoReconnect(true);
	yield();

	{
		HeapSelectIram iram;
		// ArduinoOTA.setHostname(HOSTNAME);
		// ArduinoOTA.begin();
		// MDNS.begin(HOSTNAME);
	}

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
		eeprom_settings = {};
		save_settings();
	}

	{
		HeapSelectIram iram;
		// connect SD card
		// "Note that even if you don’t use the hardware SS pin, it must be left as an output or the SD library won’t work."
		pinMode(SS, OUTPUT);
		bool sdConnectResult = card.begin(SD_CONFIG);
		if (!sdConnectResult) {
			debug_print(F("Connection to SD card FAILED. Error code:"));
			debug_print(card.sdErrorCode());
			debug_print(F("Data:"));
			debug_print(card.sdErrorData());
			// ESP.reset();
		} else {
			debug_print(F("SD ok."));
		}
		yield();
	}

	debug_print(F("Trying to setup audio..."));
	// audioLogger = &DebugManager::the();
	AudioManager::the();

	display.firstPage();
	do {
		display.setFont(MAIN_FONT);
		// text should not override display data behind characters, this enables "transparency"
		display.setFontMode(1);
	} while (display.nextPage());

	TimeManager::the().update_if_needed();

	current_menu = create_menu_structure();

	// initialize timing variables
	button_change_time = previous_loop_time = drawTime = millis();
}

void loop()
{
	// the timing millisecond counter that rolls around every minute or so
	auto current_loop_time = millis();
	yield();

	// ArduinoOTA.handle();
	// yield();
	// MDNS.update();
	yield();
	TimeManager::the().update_if_needed();
	yield();
	// DebugManager::the().handle();

	// read buttons, some bit magic here
	uint8_t buttons = 0x0f & (((analogRead(PIN_BUTTON_UPDOWN) > 750) << BUTTON_UP_BIT) | ((analogRead(PIN_BUTTON_UPDOWN) < 350) << BUTTON_DOWN_BIT) | ((~digitalRead(PIN_BUTTON_RIGHT) & 1) << BUTTON_RIGHT_BIT) | ((~digitalRead(PIN_BUTTON_LEFT) & 1) << BUTTON_LEFT_BIT));
	{
		HeapSelectIram iram;

		// if a button is held and the time since button change exceeds the hold time "delay"...
		if ((current_loop_time - button_change_time > BUTTON_HOLD_DELAY) && (buttons != 0)) {
			// decrease delta to next simulated button press by loop delta
			button_hold_time_delta -= current_loop_time - previous_loop_time;
		} else {
			// else reset delta
			button_hold_time_delta = BUTTON_HOLD_REPEAT_DELAY;
		}

		// temporary storage for a possibly different new menu
		Menu* newMenu = current_menu;
		// handle buttons
		if (buttons != last_buttons || (button_hold_time_delta < 0)) {
			newMenu = newMenu->handle_button(buttons);
		}

		// any button state is different: update last button time
		if (buttons != last_buttons) {
			button_change_time = current_loop_time;
			button_hold_time_delta = BUTTON_HOLD_REPEAT_DELAY;
		}
		// reset time to next simulated button press if a button press was just simulated
		if (button_hold_time_delta < 0) {
			button_hold_time_delta += BUTTON_HOLD_REPEAT_DELAY;
		}

		yield();

		// draw if menu changed due to buttons
		if (current_menu != newMenu) {
			newMenu = newMenu->draw_menu(&display, current_loop_time - drawTime);
		}
		// draw if menu wants to refresh
		else if (newMenu->should_refresh(current_loop_time - previous_loop_time)) {
			yield();
			current_menu = newMenu->draw_menu(&display, current_loop_time - drawTime);
			drawTime = current_loop_time;
		}

		// store new menu
		current_menu = newMenu;
	}

	// TODO: light sleep regularly crashes the ESP8266, and I don't quite know why.
	// The entire light sleep setup itself is rather finnicky in the first place, and there's zero good documentation on it.
	// Since the clock can restore its state with ease after a power cycle (usually needing <10s to reconnect to Wifi and fetching NTP),
	// this is not really an issue, but it annoyingly makes the screen turn on sporadically.
	if (current_loop_time - button_change_time > eeprom_settings.sleep_time && !AudioManager::the().is_playing()) {
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
		TimeManager::the().force_update();
		button_change_time = current_loop_time = millis();
	}
	yield();

	// store state and time
	last_buttons = buttons;
	previous_loop_time = current_loop_time;
}
