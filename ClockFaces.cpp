#include "ClockFaces.h"
#include "Definitions.h"
#include "DisplayUtils.h"
#include "graphics.h"

namespace ClockFaces {

static uint8_t get_center(uint8_t start, uint8_t size)
{
	return start + size / 2;
}

void basic_digital(Display* display, ace_time::ZonedDateTime* time, double,
	uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
	char time_text[6];
	snprintf_P(time_text, sizeof(time_text), PSTR("%02u:%02u"), time->hour(), time->minute());
	yield();

	display->setFont(CLOCK_FONT);
	uint16_t text_width = display->getUTF8Width(time_text);
	yield();

	uint16_t text_start = text_width > width ? x0 : (width - text_width) / 2 + x0;
	display->drawUTF8(text_start, (height + CLOCK_FONT_HEIGHT) / 2 + y0, time_text);
	yield();
}

void digital_with_seconds(Display* display, ace_time::ZonedDateTime* time, double,
	uint8_t x0, uint8_t y0, uint8_t width,
	uint8_t height)
{

	char time_text[6];
	snprintf_P(time_text, sizeof(time_text), PSTR("%02u:%02u"), time->hour(), time->minute());
	yield();

	display->setFont(CLOCK_FONT);
	uint16_t text_width = display->getUTF8Width(time_text);
	uint16_t text_start = text_width > width ? x0 : (width - text_width) / 2 + x0;
	display->drawUTF8(text_start, (height + CLOCK_FONT_HEIGHT) / 2 + y0, time_text);

	char second_text[3];
	snprintf_P(second_text, sizeof(second_text), PSTR("%02u"), time->second());
	yield();

	display->setFont(MAIN_FONT);
	display->drawUTF8(text_start + text_width,
		(height + CLOCK_FONT_HEIGHT) / 2 + y0, second_text);
	yield();
}

void basic_analog(Display* display, ace_time::ZonedDateTime* time, double second_fractions,
	uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
	const double hour = time->hour() + time->minute() / 60.0d,
				 minute = time->minute() + time->second() / 60.0d,
				 second = time->second() + second_fractions;
	const uint16_t center_x = get_center(x0, width),
				   center_y = get_center(y0, height);

	// draw 12 line segments representing hours
	for (double i = 0; i < TWO_PI; i += PI_DIV6) {
		const double inner_x = (cos(i - HALF_PI) * (ANALOG_CLOCK_FACE_SIZE / 2 - ANALOG_CLOCK_FACE_LINE_LENGTH)),
					 inner_y = (sin(i - HALF_PI) * (ANALOG_CLOCK_FACE_SIZE / 2 - ANALOG_CLOCK_FACE_LINE_LENGTH)),
					 outerX = (cos(i - HALF_PI) * ANALOG_CLOCK_FACE_SIZE / 2),
					 outerY = (sin(i - HALF_PI) * ANALOG_CLOCK_FACE_SIZE / 2);

		yield();
		display->drawLine(inner_x + center_x, inner_y + center_y, outerX + center_x,
			outerY + center_y);
	}

	const double secondAngle = (second / 60.0d) * TWO_PI - HALF_PI,
				 minuteAngle = (minute / 60.0d) * TWO_PI - HALF_PI,
				 hourAngle = ((hour >= 12 ? hour - 12 : hour) / 12.0d) * TWO_PI - HALF_PI;
	const double secOuterX = cos(secondAngle) * (ANALOG_CLOCK_FACE_SIZE / 2 - ANALOG_CLOCK_FACE_LINE_LENGTH - 2),
				 secOuterY = sin(secondAngle) * (ANALOG_CLOCK_FACE_SIZE / 2 - ANALOG_CLOCK_FACE_LINE_LENGTH - 2);

	display->drawLine(center_x, center_y, center_x + secOuterX, center_y + secOuterY);

	yield();
	display->drawLine(center_x, center_y,
		cos(minuteAngle) * ANALOG_CLOCK_FACE_MINUTE_LENGTH + center_x,
		sin(minuteAngle) * ANALOG_CLOCK_FACE_MINUTE_LENGTH + center_y);

	yield();
	display->drawLine(center_x, center_y,
		cos(hourAngle) * ANALOG_CLOCK_FACE_HOUR_LENGTH + center_x,
		sin(hourAngle) * ANALOG_CLOCK_FACE_HOUR_LENGTH + center_y);
}

struct Point {
	int8_t x;
	int8_t y;
};
static_assert(sizeof(Point) == sizeof(uint16_t));

// adjacent point for drawing a double thickness line
const Point adjacency[] PROGMEM = {
	{ 1, 0 },
	{ 1, 1 },
	{ 0, 1 },
	{ -1, 1 },
	{ -1, 0 },
	{ -1, -1 },
	{ 0, -1 },
	{ 1, -1 },
};

Point adjacent_point_for(double angle)
{
	const int8_t index = static_cast<int8_t>(round(angle / QUARTER_PI) + 8) % 8;
	return adjacency[index];
}

void modern_analog(Display* display, ace_time::ZonedDateTime* time, double second_fractions,
	uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
	const double hour = time->hour() + time->minute() / 60.0d,
				 minute = time->minute() + time->second() / 60.0d,
				 second = time->second() + second_fractions;
	const uint16_t center_x = get_center(x0, width),
				   center_y = get_center(y0, height);

	yield();
	display->drawDisc(center_x, center_y, ANALOG_CLOCK_FACE_LINE_LENGTH / 2.);

	display->setFont(u8g2_font_mozart_nbp_tf);
	for (size_t i = 0; i < 12; i++) {
		const double angle = i * PI_DIV6;
		const double outerX = (cos(angle - HALF_PI) * ANALOG_CLOCK_FACE_SIZE / 2),
					 outerY = (sin(angle - HALF_PI) * ANALOG_CLOCK_FACE_SIZE / 2);
		yield();
		char number[3];
		snprintf_P(number, sizeof(number), PSTR("%d"), i == 0 ? 12 : i);
		const auto width = display->getUTF8Width(number);
		display->drawUTF8(center_x + outerX - width / 2 + 1, center_y + outerY + 7 / 2, number);
	}

	const double secondAngle = (second / 60.0d) * TWO_PI - HALF_PI,
				 minuteAngle = (minute / 60.0d) * TWO_PI - HALF_PI,
				 hourAngle = ((hour >= 12 ? hour - 12 : hour) / 12.0d) * TWO_PI - HALF_PI;

	const uint8_t minute_x = cos(minuteAngle) * ANALOG_CLOCK_FACE_MINUTE_LENGTH + center_x,
				  minute_y = sin(minuteAngle) * ANALOG_CLOCK_FACE_MINUTE_LENGTH + center_y;
	const auto adjacent_minute = adjacent_point_for(minuteAngle);

	// display->drawLine(center_x, center_y, minute_x, minute_y);
	display->drawLine(center_x, center_y, minute_x + adjacent_minute.x, minute_y + adjacent_minute.y);
	yield();
	display->drawLine(center_x, center_y,
		cos(hourAngle) * ANALOG_CLOCK_FACE_HOUR_LENGTH + center_x,
		sin(hourAngle) * ANALOG_CLOCK_FACE_HOUR_LENGTH + center_y);
}

void rotating_segment_analog(Display* display, ace_time::ZonedDateTime* time, double second_fractions,
	uint8_t x0, uint8_t y0, uint8_t width,
	uint8_t height)
{
	const double hour = time->hour() + time->minute() / 60.0d,
				 minute = time->minute() + time->second() / 60.0d,
				 second = time->second() + second_fractions;
	const uint16_t center_x = get_center(x0, width),
				   center_y = get_center(y0, height);

	const double secondAngle = second * TWO_PI / 60.0d,
				 minuteAngle = minute * TWO_PI / 60.0d,
				 hourAngle = (hour >= 12 ? hour - 12 : hour) * TWO_PI / 12.0d;

	// the offsets need to be as smooth as possible, but not time-precise
	// make them dependent on internal milliseconds, which will only glitch out
	// every 50 days or so
	const double secondOffset = fmod(millis() / 1000.0d / VRAND0, TWO_PI);
	const double minuteOffset = fmod(-millis() / 1400.0d / VRAND1, TWO_PI);
	const double hourOffset = fmod(millis() / 2600.0d / VRAND2, TWO_PI);

	yield();
	draw_arc(display, center_x, center_y, height / 2 - LINESEP * 2, secondOffset,
		secondOffset + secondAngle);
	yield();
	draw_arc(display, center_x, center_y, height / 2 - LINESEP * 4, minuteOffset,
		minuteOffset + minuteAngle);
	yield();
	draw_arc(display, center_x, center_y, height / 2 - LINESEP * 8, hourOffset,
		hourOffset + hourAngle);
}

void binary(Display* display, ace_time::ZonedDateTime* time, double, uint8_t x0,
	uint8_t y0, uint8_t width, uint8_t height)
{
	const uint8_t hour = time->hour(), minute = time->minute(),
				  second = time->second();
	// vertical position of each row
	const uint16_t hourpos = (height - BINARY_CLOCK_FACE_BOX_SIZE) / 2 - (BINARY_CLOCK_FACE_BOX_SIZE + BINARY_CLOCK_FACE_BOX_SPACING) + y0,
				   minutepos = (height - BINARY_CLOCK_FACE_BOX_SIZE) / 2 + y0,
				   secondpos = (height - BINARY_CLOCK_FACE_BOX_SIZE) / 2 + (BINARY_CLOCK_FACE_BOX_SIZE + BINARY_CLOCK_FACE_BOX_SPACING) + y0;
	// maximum of 6 bits for minute/second, 5 bits for hour, go with one
	// 6-bit-loop
	for (uint8_t bit = 0; bit < 6; ++bit) {
		yield();
		if (hour & (1 << bit)) {
			display->drawBox(bit * (BINARY_CLOCK_FACE_BOX_SIZE + BINARY_CLOCK_FACE_BOX_SPACING) + BINARY_CLOCK_FACE_BOX_SPACING + x0,
				hourpos, BINARY_CLOCK_FACE_BOX_SIZE, BINARY_CLOCK_FACE_BOX_SIZE);
		}
		if (minute & (1 << bit)) {
			display->drawBox(bit * (BINARY_CLOCK_FACE_BOX_SIZE + BINARY_CLOCK_FACE_BOX_SPACING) + BINARY_CLOCK_FACE_BOX_SPACING + x0,
				minutepos, BINARY_CLOCK_FACE_BOX_SIZE, BINARY_CLOCK_FACE_BOX_SIZE);
		}
		if (second & (1 << bit)) {
			display->drawBox(bit * (BINARY_CLOCK_FACE_BOX_SIZE + BINARY_CLOCK_FACE_BOX_SPACING) + BINARY_CLOCK_FACE_BOX_SPACING + x0,
				secondpos, BINARY_CLOCK_FACE_BOX_SIZE, BINARY_CLOCK_FACE_BOX_SIZE);
		}
	}
}

void day_seconds_binary(Display* display, ace_time::ZonedDateTime* time, double,
	uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
	const uint32_t second = time->second() + time->minute() * 60 + time->hour() * 60 * 24;

	uint16_t current_y = (height - BINARY_CLOCK_FACE_BOX_SIZE) / 2;
	// maximum of 17 bits for second of day
	for (uint16_t current_x = BINARY_CLOCK_FACE_BOX_SPACING, bit = 0; bit < 17;
		 ++bit, current_x += BINARY_CLOCK_FACE_BOX_SPACING + BINARY_CLOCK_FACE_BOX_SIZE) {
		yield();
		// "line wrap" if boxes would overshoot the screen width
		if (current_x + BINARY_CLOCK_FACE_BOX_SIZE > width) {
			current_x = BINARY_CLOCK_FACE_BOX_SPACING;
			current_y += BINARY_CLOCK_FACE_BOX_SIZE + BINARY_CLOCK_FACE_BOX_SPACING;
		}
		if (second & (1 << bit)) {
			display->drawBox(current_x + x0, current_y + y0, BINARY_CLOCK_FACE_BOX_SIZE,
				BINARY_CLOCK_FACE_BOX_SIZE);
		}
	}
}

}
