#include "RealtimeTimer.h"
#include "Arduino.h"

void RealtimeTimer::attach(timercallback callback, uint64_t interval_microseconds)
{
	constexpr double div1_max_microseconds = (1 << 23) / 80.;
	constexpr double div16_max_microseconds = (1 << 23) / 5.;
	constexpr double div256_max_microseconds = ((1 << 23) / 80.) * 256.;

	TIM_DIV_ENUM divider;
	uint32_t ticks;
	if (interval_microseconds < div16_max_microseconds) {
		divider = TIM_DIV16;
		ticks = interval_microseconds * 5;
	} else if (interval_microseconds < div1_max_microseconds) {
		divider = TIM_DIV1;
		ticks = interval_microseconds * 80;
	} else if (interval_microseconds < div256_max_microseconds) {
		divider = TIM_DIV256;
		ticks = (interval_microseconds * 80) / 256;
	} else {
		divider = TIM_DIV256;
		ticks = 0xffffffff;
	}

	timer1_attachInterrupt(callback);
	timer1_enable(divider, TIM_EDGE, TIM_LOOP);
	timer1_write(ticks);
}

void RealtimeTimer::detach()
{
	timer1_detachInterrupt();
}
