#include "ClockFaces.h"
#include "Debug.h"
#include "Definitions.h"
#include "DisplayUtils.h"
#include "LUTMath.h"
#include "graphics.h"

namespace ClockFaces {

static uint8_t get_center(uint8_t start, uint8_t size)
{
	return start + size / 2;
}

void basic_digital(Display* display, ace_time::ZonedDateTime* time, double,
	uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
	auto hour = time->hour();
	if (eeprom_settings.clock_settings.time_format != TimeFormat::Hours24) {
		hour %= 12;
		if (hour == 0)
			hour = 12;
	}

	char time_text[6];
	snprintf_P(time_text, sizeof(time_text), PSTR("%02u:%02u"), hour, time->minute());
	yield();

	display->setFont(CLOCK_FONT);
	uint16_t text_width = display->getUTF8Width(time_text);

	char second_text[3];
	if (eeprom_settings.clock_settings.show_seconds) {
		snprintf_P(second_text, sizeof(second_text), PSTR("%02u"), time->second());
		display->setFont(MAIN_FONT);
		text_width += display->getUTF8Width(second_text) * 2;
		display->setFont(CLOCK_FONT);
	}

	yield();
	uint16_t text_start = text_width > width ? x0 : (width - text_width) / 2 + x0;
	display->drawUTF8(text_start, (height + CLOCK_FONT_HEIGHT) / 2 + y0, time_text);
	text_start += display->getUTF8Width(time_text) + LEFT_TEXT_MARGIN;

	if (eeprom_settings.clock_settings.show_seconds) {
		yield();
		display->setFont(MAIN_FONT);
		display->drawUTF8(text_start,
			(height + CLOCK_FONT_HEIGHT) / 2 + y0, second_text);
		text_start += display->getUTF8Width(second_text) + LEFT_TEXT_MARGIN;
	}

	if (eeprom_settings.clock_settings.time_format == TimeFormat::Hours12AmPm) {
		yield();
		display->setFont(MAIN_FONT);
		display->drawUTF8(text_start,
			(height + CLOCK_FONT_HEIGHT) / 2 + y0, time->hour() < 12 ? am : pm);
	}
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
		const double inner_x = (cos_lut(i - HALF_PI) * (ANALOG_CLOCK_FACE_SIZE / 2 - ANALOG_CLOCK_FACE_LINE_LENGTH)),
					 inner_y = (sin_lut(i - HALF_PI) * (ANALOG_CLOCK_FACE_SIZE / 2 - ANALOG_CLOCK_FACE_LINE_LENGTH)),
					 outerX = (cos_lut(i - HALF_PI) * ANALOG_CLOCK_FACE_SIZE / 2),
					 outerY = (sin_lut(i - HALF_PI) * ANALOG_CLOCK_FACE_SIZE / 2);

		yield();
		display->drawLine(inner_x + center_x, inner_y + center_y, outerX + center_x,
			outerY + center_y);
	}

	const double secondAngle = (second / 60.0d) * TWO_PI - HALF_PI,
				 minuteAngle = (minute / 60.0d) * TWO_PI - HALF_PI,
				 hourAngle = ((hour >= 12 ? hour - 12 : hour) / 12.0d) * TWO_PI - HALF_PI;
	const double secOuterX = cos_lut(secondAngle) * (ANALOG_CLOCK_FACE_SIZE / 2 - ANALOG_CLOCK_FACE_LINE_LENGTH - 2),
				 secOuterY = sin_lut(secondAngle) * (ANALOG_CLOCK_FACE_SIZE / 2 - ANALOG_CLOCK_FACE_LINE_LENGTH - 2);

	if (eeprom_settings.clock_settings.show_seconds)
		display->drawLine(center_x, center_y, center_x + secOuterX, center_y + secOuterY);

	yield();
	display->drawLine(center_x, center_y,
		cos_lut(minuteAngle) * ANALOG_CLOCK_FACE_MINUTE_LENGTH + center_x,
		sin_lut(minuteAngle) * ANALOG_CLOCK_FACE_MINUTE_LENGTH + center_y);

	yield();
	display->drawLine(center_x, center_y,
		cos_lut(hourAngle) * ANALOG_CLOCK_FACE_HOUR_LENGTH + center_x,
		sin_lut(hourAngle) * ANALOG_CLOCK_FACE_HOUR_LENGTH + center_y);
}

void modern_analog(Display* display, ace_time::ZonedDateTime* time, double second_fractions,
	uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
	const auto inner_radius = ANALOG_CLOCK_FACE_LINE_LENGTH / 2.;

	const double hour = time->hour() + time->minute() / 60.0d,
				 minute = time->minute() + time->second() / 60.0d,
				 second = time->second() + second_fractions;
	const uint16_t center_x = get_center(x0, width),
				   center_y = get_center(y0, height);

	yield();
	display->drawDisc(center_x, center_y, inner_radius);

	display->setFont(u8g2_font_mozart_nbp_tf);
	for (size_t i = 0; i < 12; i++) {
		const double angle = i * PI_DIV6;
		const double outerX = (cos_lut(angle - HALF_PI) * ANALOG_CLOCK_FACE_SIZE / 2),
					 outerY = (sin_lut(angle - HALF_PI) * ANALOG_CLOCK_FACE_SIZE / 2);
		yield();
		const char* number = hour_names_modern[i];
		const auto width = display->getUTF8Width(number);
		display->drawUTF8(center_x + outerX - width / 2 + 1, center_y + outerY + 7 / 2, number);
	}

	const double secondAngle = (second / 60.0d) * TWO_PI - HALF_PI,
				 minuteAngle = (minute / 60.0d) * TWO_PI - HALF_PI,
				 hourAngle = ((hour >= 12 ? hour - 12 : hour) / 12.0d) * TWO_PI - HALF_PI;

	const uint8_t minute_x = cos_lut(minuteAngle) * (ANALOG_CLOCK_FACE_MINUTE_LENGTH + inner_radius) + center_x,
				  minute_y = sin_lut(minuteAngle) * (ANALOG_CLOCK_FACE_MINUTE_LENGTH + inner_radius) + center_y,
				  hour_x = cos_lut(hourAngle) * (ANALOG_CLOCK_FACE_HOUR_LENGTH + inner_radius) + center_x,
				  hour_y = sin_lut(hourAngle) * (ANALOG_CLOCK_FACE_HOUR_LENGTH + inner_radius) + center_y,
				  // start position of the hands on the other side of the center point
		counter_hand_minute_x = cos_lut(minuteAngle) * (ANALOG_CLOCK_FACE_MINUTE_LENGTH + inner_radius) + center_x;

	display->drawLine(center_x, center_y, minute_x, minute_y);
	yield();
	draw_stroked_line(display, center_x, center_y, hour_x, hour_y, hourAngle + HALF_PI, 3);

	if (eeprom_settings.clock_settings.show_seconds) {
		const double second_end_x = cos_lut(secondAngle) * (ANALOG_CLOCK_FACE_SIZE / 2 - ANALOG_CLOCK_FACE_LINE_LENGTH - 2),
					 second_end_y = sin_lut(secondAngle) * (ANALOG_CLOCK_FACE_SIZE / 2 - ANALOG_CLOCK_FACE_LINE_LENGTH - 2);
		display->drawLine(center_x, center_y, center_x + second_end_x, center_y + second_end_y);
	}

	display->setDrawColor(0);
	display->drawPixel(center_x, center_y);
	display->setDrawColor(1);
}

void retro_analog(Display* display, ace_time::ZonedDateTime* time, double second_fractions,
	uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
	const auto inner_radius = ANALOG_CLOCK_FACE_LINE_LENGTH / 3.;

	const double hour = time->hour() + time->minute() / 60.0d,
				 minute = time->minute() + time->second() / 60.0d,
				 second = time->second() + second_fractions;
	const uint16_t center_x = get_center(x0, width),
				   center_y = get_center(y0, height);

	yield();
	display->drawDisc(center_x, center_y, inner_radius);

	display->setFont(TINY_FONT);
	for (size_t i = 0; i < 12; i++) {
		const double angle = i * PI_DIV6;
		const double outerX = (cos_lut(angle - HALF_PI) * ANALOG_CLOCK_FACE_SIZE / 2),
					 outerY = (sin_lut(angle - HALF_PI) * ANALOG_CLOCK_FACE_SIZE / 2);
		yield();
		const char* number = hour_names_retro[i];
		const auto width = display->getUTF8Width(number);
		display->drawUTF8(center_x + outerX - width / 2 + 1, center_y + outerY + 7 / 2, number);
	}

	const double secondAngle = (second / 60.0d) * TWO_PI - HALF_PI,
				 minuteAngle = (minute / 60.0d) * TWO_PI - HALF_PI,
				 hourAngle = ((hour >= 12 ? hour - 12 : hour) / 12.0d) * TWO_PI - HALF_PI;
	constexpr double DECORATION_ANGLE_OFFSET = PI_FACTOR<1, 15>;

	const uint8_t minute_x = cos_lut(minuteAngle) * (ANALOG_CLOCK_FACE_MINUTE_LENGTH + inner_radius) + center_x,
				  minute_y = sin_lut(minuteAngle) * (ANALOG_CLOCK_FACE_MINUTE_LENGTH + inner_radius) + center_y,
				  hour_x = cos_lut(hourAngle) * (ANALOG_CLOCK_FACE_HOUR_LENGTH + inner_radius) + center_x,
				  hour_y = sin_lut(hourAngle) * (ANALOG_CLOCK_FACE_HOUR_LENGTH + inner_radius) + center_y,
				  minute_opposite_x = -cos_lut(minuteAngle) * (ANALOG_CLOCK_FACE_COUNTER_HAND_LENGTH + inner_radius) + center_x,
				  minute_opposite_y = -sin_lut(minuteAngle) * (ANALOG_CLOCK_FACE_COUNTER_HAND_LENGTH + inner_radius) + center_y,
				  hour_opposite_x = -cos_lut(hourAngle) * (ANALOG_CLOCK_FACE_COUNTER_HAND_LENGTH + inner_radius) + center_x,
				  hour_opposite_y = -sin_lut(hourAngle) * (ANALOG_CLOCK_FACE_COUNTER_HAND_LENGTH + inner_radius) + center_y,
				  minute_left_side_x = cos_lut(minuteAngle + DECORATION_ANGLE_OFFSET) * (ANALOG_CLOCK_FACE_MINUTE_LENGTH - ANALOG_CLOCK_FACE_LINE_LENGTH) + center_x,
				  minute_left_side_y = sin_lut(minuteAngle + DECORATION_ANGLE_OFFSET) * (ANALOG_CLOCK_FACE_MINUTE_LENGTH - ANALOG_CLOCK_FACE_LINE_LENGTH) + center_y,
				  minute_right_side_x = cos_lut(minuteAngle - DECORATION_ANGLE_OFFSET) * (ANALOG_CLOCK_FACE_MINUTE_LENGTH - ANALOG_CLOCK_FACE_LINE_LENGTH) + center_x,
				  minute_right_side_y = sin_lut(minuteAngle - DECORATION_ANGLE_OFFSET) * (ANALOG_CLOCK_FACE_MINUTE_LENGTH - ANALOG_CLOCK_FACE_LINE_LENGTH) + center_y,
				  hour_left_side_x = cos_lut(hourAngle + DECORATION_ANGLE_OFFSET * 2) * (ANALOG_CLOCK_FACE_HOUR_LENGTH - ANALOG_CLOCK_FACE_LINE_LENGTH / 2.0) + center_x,
				  hour_left_side_y = sin_lut(hourAngle + DECORATION_ANGLE_OFFSET * 2) * (ANALOG_CLOCK_FACE_HOUR_LENGTH - ANALOG_CLOCK_FACE_LINE_LENGTH / 2.0) + center_y,
				  hour_right_side_x = cos_lut(hourAngle - DECORATION_ANGLE_OFFSET * 2) * (ANALOG_CLOCK_FACE_HOUR_LENGTH - ANALOG_CLOCK_FACE_LINE_LENGTH / 2.0) + center_x,
				  hour_right_side_y = sin_lut(hourAngle - DECORATION_ANGLE_OFFSET * 2) * (ANALOG_CLOCK_FACE_HOUR_LENGTH - ANALOG_CLOCK_FACE_LINE_LENGTH / 2.0) + center_y;

	display->drawLine(minute_opposite_x, minute_opposite_y, minute_x, minute_y);
	display->drawLine(minute_right_side_x, minute_right_side_y, minute_x, minute_y);
	display->drawLine(minute_x, minute_y, minute_left_side_x, minute_left_side_y);
	yield();
	display->drawLine(hour_opposite_x, hour_opposite_y, hour_x, hour_y);
	display->drawLine(hour_right_side_x, hour_right_side_y, hour_x, hour_y);
	display->drawLine(hour_x, hour_y, hour_left_side_x, hour_left_side_y);

	// display->setDrawColor(0);
	// display->drawPixel(center_x, center_y);
	// display->setDrawColor(1);
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

	// TODO: 24h format hour angle is not correct
	const double secondAngle = second * TWO_PI / 60.0d,
				 minuteAngle = minute * TWO_PI / 60.0d,
				 hourAngle = (eeprom_settings.clock_settings.time_format == TimeFormat::Hours24 ? hour : (hour >= 12 ? hour - 12 : hour))
		* TWO_PI / 12.0d;

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
