#include <Arduino.h>

/** Actual realtime timer interrupt. For some reason this doesn't exist yet:
	- Ticker uses the OS timer feature, which is just using the cooperative multitasking via yield(). Not realtime capable.
	- Various libraries use timer 0 for some fucking reason, which disables the RTOS functionality and will get you killed by the watchdog.

	Note: This thing is horribly broken, even though it technically should work and WiFi shouldn't be using timer 1.
	I have no idea what's going on and give up for now.
*/
class RealtimeTimer {
public:
	static void attach(timercallback callback, uint64_t interval_microseconds);
	static void detach();

private:
	RealtimeTimer() = delete;
};
