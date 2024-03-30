#pragma once

#include "Definitions.h"
#include "DisplayUtils.h"
#include "Globals.h"
#include "PrintString.h"
#include "Settings.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <HardwareSerial.h>

class DebugManager : public Print {
public:
	static DebugManager& the();
	DebugManager();

	virtual size_t write(uint8_t) override;
	virtual size_t write(const uint8_t* buffer, size_t size) override;

	void handle();

private:
	// Singleton instance
	static std::unique_ptr<DebugManager> instance;

	WiFiServer log_server { 1000 };
	std::vector<WiFiClient> log_clients {};
};

template <typename Printable>
inline void debug_print(Printable text)
{
	DebugManager::the().println(text);
}

// template <typename... Arguments>
// inline void debug_print(__FlashStringHelper const* format, Arguments&&... arguments)
// {
// 	// FIXME: compromise between non-infinite printf length
// 	// and 10K of ram use for C++ standard library formatting functionality.
// 	char output[256] {};
// 	snprintf_P(output, sizeof(output), (PGM_P)format, std::forward<Arguments>(arguments)...);
// 	DebugManager::the().println(output);
// }
