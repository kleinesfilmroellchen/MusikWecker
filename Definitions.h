/**
   Definition header.
*/

#pragma once

// enable serial with USE_SERIAL=1
// this may break the I2S DAC!
#define USE_SERIAL 0

// F = full framebuffer
// 1 = small partial framebuffer
// 2 = larger partial framebuffer
#define FRAMEBUFFER_SIZE '2'

#include "SdFatConfig.h"
#include "ssid.h"
#include <Arduino.h>
#include <SdFat.h>
#include <U8g2lib.h>
#include <stdint.h>

template <uint64_t numerator, uint64_t denominator, typename Floating = double>
constexpr Floating PI_FACTOR = static_cast<Floating>(numerator) * static_cast<Floating>(PI) / static_cast<Floating>(denominator);

// 30 degrees = π/6 radians
constexpr double PI_DIV6 = PI_FACTOR<1, 6>;
// 90 degrees = π/4 radians
constexpr double QUARTER_PI = PI_FACTOR<1, 4>;
// 135 degrees = 3π/4 radians
constexpr double THREE_QUARTER_PI = PI_FACTOR<3, 4>;

// chip select pin for sd card
constexpr uint8_t PIN_SD_CS = TX;
const SdSpiConfig SD_CONFIG = {
	PIN_SD_CS,
	DEDICATED_SPI,
	// Board is pretty glitchy above 20MHz even though any SD card should be able to do 40MHz.
	// If the SD card decides to act up again, decrease this to 20MHz and slowly increase again as long as it's stable.
	SD_SCK_MHZ(8),
};

// screen size
constexpr uint16_t SCREEN_WIDTH = 128;
constexpr uint16_t SCREEN_HEIGHT = 64;

// You may have to adjust this for other displays and connections
// This is a SSD1306-based 128 by 64 monochrome OLED which runs on hardware I²C address 0x3C (specified by VCOMH0) without a proper reset pin.
// We could use 128 bytes page buffer if other libraries need a decent amount of memory, change FRAMEBUFFER_SIZE to "1" or "2"
#if FRAMEBUFFER_SIZE == 'F'
using Display = U8G2_SSD1306_128X64_VCOMH0_F_HW_I2C;
#elif FRAMEBUFFER_SIZE == '1'
using Display = U8G2_SSD1306_128X64_VCOMH0_1_HW_I2C;
#elif FRAMEBUFFER_SIZE == '2'
using Display = U8G2_SSD1306_128X64_VCOMH0_2_HW_I2C;
#endif

// Hardware I2C clock speed for the display.
// Even though most displays are run at 400KHz by default, many can be overdriven much higher.
// Increasing this value reduces audio glitches, since the I2C DMA interferes with I2S DMA.
// If the display is broken, reduce this value as much as necessary.
constexpr uint32_t DISPLAY_CLOCK_SPEED = 2'000'000;

//// font stuff
// Main font used for UI text. Any 10-12 px font will allow 4-5 lines of text.
// good fonts:
// u8g2_font_cu12_he (12px height)
// u8g2_font_t0_15_te (10px height)
// u8g2_font_mercutio_basic_nbp_t_all (10px height)
// u8g2_font_unifont_he (12px height)
constexpr auto MAIN_FONT = u8g2_font_mercutio_basic_nbp_t_all;
// Height of the main font in pixels
constexpr uint8_t MAIN_FONT_SIZE = 10;
// Font used for digital clock faces; very large (~1/2 screen height)
constexpr auto CLOCK_FONT = u8g2_font_fur30_tn;
constexpr uint8_t CLOCK_FONT_HEIGHT = 30;
// Tiny font (4-6px height) for debug text and other small elements.
constexpr auto TINY_FONT = u8g2_font_tom_thumb_4x6_t_all;
constexpr uint8_t TINY_FONT_HEIGHT = 5;
// Script font used for various non-UI prose text.
constexpr auto SCRIPT_FONT = u8g2_font_helvR10_te;
// spacing between clock face status symbols
constexpr uint8_t SYMBOL_SPACING = 1;
// Size of the gap between lines. Increase this if lines cut into each other (especially chars like "g")
constexpr uint8_t LINESEP = 3;
// Height of a single line, depends on font size and line separator gap.
constexpr uint8_t LINE_HEIGHT = (MAIN_FONT_SIZE + LINESEP - 1);
// Computed number of lines, depends on screen and line height.
constexpr uint8_t LINE_COUNT = static_cast<uint8_t>(SCREEN_HEIGHT / LINE_HEIGHT);
// How many pixels to shift all text to the right. This makes inverted text look cleaner if greater than 0.
constexpr uint8_t LEFT_TEXT_MARGIN = 1;
// random numbers 1-10 for visual stuff
constexpr double VRAND0 = 2.8876;
constexpr double VRAND1 = 6.209;
constexpr double VRAND2 = 7.1195;
constexpr double VRAND3 = 4.973;

// menuing buttons
constexpr uint8_t PIN_BUTTON_UPDOWN = 0; // prev/next on one analog pin, voltage divider hack (see button reading logic)
constexpr uint8_t PIN_BUTTON_RIGHT = D0; // enter
constexpr uint8_t PIN_BUTTON_LEFT = D3; // back

// button bits, button constants depend on them
constexpr uint8_t BUTTON_UP_BIT = 0;
constexpr uint8_t BUTTON_DOWN_BIT = 1;
constexpr uint8_t BUTTON_RIGHT_BIT = 2;
constexpr uint8_t BUTTON_LEFT_BIT = 3;

// button constants for button press handling
constexpr uint8_t BUTTON_UP = 0b1 << BUTTON_UP_BIT;
constexpr uint8_t BUTTON_DOWN = 0b1 << BUTTON_DOWN_BIT;
constexpr uint8_t BUTTON_RIGHT = 0b1 << BUTTON_RIGHT_BIT;
constexpr uint8_t BUTTON_LEFT = 0b1 << BUTTON_LEFT_BIT;

// ms delay after which a button hold is considered a repeated press; e.g. for fast scrolling or value adjustment
constexpr uint16_t BUTTON_HOLD_DELAY = 700;
// ms interval between "simulated" button presses that are triggered on button hold, determines percieved scroll speed
constexpr uint16_t BUTTON_HOLD_REPEAT_DELAY = 100;
// ms interval after which a clock preview is shown in the clock settings menu
constexpr uint16_t CLOCK_PREVIEW_DELAY = 5000;

// temporary hard-coded time server, user will later be able to choose
constexpr char const* TEMP_TIME_SERVER = "europe.pool.ntp.org";
// ntp update interval at 30 minutes
constexpr uint16_t TEMP_NTP_UPDATE_INTERVAL = 60000;
// for testing
// constexpr uint16_t TEMP_NTP_UPDATE_INTERVAL = 5000;
// cache size for AceTime timezone manager
constexpr uint16_t TIME_ZONE_CACHE_SIZE = 2;
// interval in which the clock screen updates (millis)
constexpr uint16_t CLOCK_UPDATE_INTERVAL = 1000 / 20;
// how long the clock sync symbol stays on screen after a ntp time server synchronisation occurred (millis)
constexpr uint16_t CLOCKSYNC_SYMBOL_DURATION = 3500;
