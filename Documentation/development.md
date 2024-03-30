# Developer Guide

Use the Arduino IDE or CLI. There's VSCode integration too!

clangd or the Microsoft C/C++ language server should work well.

Formatting with clang-format is necessary for contributions. The config is in the standard .clang-format, MusikWecker uses minimally modified WebKit style.

## Debugging and logging

Since the serial port's pins are in use for other purposes, debugging via serial is not possible. (There is an option in Definitions.h to enable serial setup and prints, but it does not work if SD card and I2S code is not disabled.) Instead, debugging via TCP has been implemented. The MusikWecker provides a TCP server on port 1000, and connecting to that TCP server allows you to receive log messages.

The TCP logging does not easily handle early boot messages, or early boot failures. Debugging those is exceptionally hard to impossible on the MusikWecker and comes down to trial and error, i.e. commenting and un-commenting code until an offending statement is found and using guesswork to determine the underlying issue. Sometimes, serial debugging is possible here; you may need to disable the SD card initialization and/or audio management initialization.

## If the Bad Apple demo is slow

(i.e. stuttery video playback, but fine audio playback)

Force all important I2S routines into IRAM, like so:

```cpp
// I2S.h in ESP8266 core libraries
// insert the IRAM_ATTR in the appropriate places:

  // from Stream
  virtual int IRAM_ATTR available();
  virtual int IRAM_ATTR read(); // Blocking, will wait for incoming data
  virtual int IRAM_ATTR peek(); // Blocking, will wait for incoming data   
  virtual void IRAM_ATTR flush();

  // from Print (see notes on write() methods below)
  virtual size_t IRAM_ATTR write(uint8_t);
  virtual size_t IRAM_ATTR write(const uint8_t *buffer, size_t size);
  virtual int IRAM_ATTR availableForWrite();

```

[An issue to enable this via a configurable option has been opened upstream](https://github.com/esp8266/Arduino/issues/9115).

## TODOs because I forget

Some of the things I still need to implement. In rough order of priority

### Must

- Bugs/QOL
	- File list doesn't reload everywhere when files are moved and deleted
	- Diagnostics page is broken
	- Move clock design page to new settings section (see below)
- The entire alarm system
	- Creating and deleting alarms
	- Alarm saving and restoring (EEPROM is fixed size; how to implement dynamic alarm clock count?)
	- Alarm settings (which are available?)
	- Executing alarms
	- Correct wakeup before an alarm runs (at least 10s to allow WiFi reconnect)
- Software volume control
- Power saving shenanigans
	- Core clock-down while we're not playing music. 160MHz is really only needed for the high SPI+I2S+audio decode workload.
	- Flash clock-down for the same reasons.
- Play music when selected in file view
- Important settings
	- Disable/enable date display
	- Configure clock format for digital clocks (12h/24h)
	- Configure date format
		- ISO
		- German short
		- German long
	- Configure NTP update interval
		- 60s
		- 5min
		- 30min
		- 24h
	- Configure NTP server

### Want

- Nicer interface for yes/no selection menus
- Web UI for configuring more complex stuff
	- Name alarms
	- Specifying 
	- Rename files
- Talk to Home Assistant for updating alarm times (since HA has an easier time receiving alarm data from Google services and the like)
- MP3 player
	- "Currently playing" page (in the clock menu?)
	- Play entire folder
	- Index all music, allow playback by artist, album, etc.

### Nice to have

- Put together a BOM for making a single PCB for this
- Design a single PCB for this
- Manufacture a single PCB for this
- World domination