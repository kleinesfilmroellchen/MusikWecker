/**Clock face functions header*/

#pragma once

#include "Definitions.h"
#include "string_constants.h"
#include <AceTime.h>
#include <U8g2lib.h>
#include <array>

namespace ClockFaces {

// size of the analog clock face
constexpr uint16_t ANALOG_CLOCK_FACE_SIZE = 55;
// length of the line segments that represent the hour markings
constexpr uint16_t ANALOG_CLOCK_FACE_LINE_LENGTH = 4;
// length of the hour hand
constexpr uint16_t ANALOG_CLOCK_FACE_HOUR_LENGTH = 10;
// length of the minute hand
constexpr uint16_t ANALOG_CLOCK_FACE_MINUTE_LENGTH = 16;
// length of the opposing piece of a hand in retro analog clocks
constexpr uint16_t ANALOG_CLOCK_FACE_COUNTER_HAND_LENGTH = 4;
// size of the boxes in binary clocks
constexpr uint16_t BINARY_CLOCK_FACE_BOX_SIZE = 8;
// spacing between the boxes in binary clocks
constexpr uint16_t BINARY_CLOCK_FACE_BOX_SPACING = 2;
// computed maximum number of boxes in a row
constexpr uint8_t BINARY_CLOCK_FACE_MAX_BOXES_PER_ROW = static_cast<uint8_t>(SCREEN_WIDTH / (BINARY_CLOCK_FACE_BOX_SIZE + BINARY_CLOCK_FACE_BOX_SPACING));

// typedef the clock face function pointer type
using ClockFace = void (*)(Display*, ace_time::ZonedDateTime*, double second_fractions, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);

/** Basic digital clock. */
void basic_digital(Display* display, ace_time::ZonedDateTime* time, double, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
/** Minimalistic analog clock without numbers. */
void basic_analog(Display* display, ace_time::ZonedDateTime* time, double, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
/** Retro analog clock with Roman numerals. */
void retro_analog(Display* display, ace_time::ZonedDateTime* time, double, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
/** Modern analog clock with numbers. */
void modern_analog(Display* display, ace_time::ZonedDateTime* time, double, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
/** Analog clock with rotating segments for each time division. */
void rotating_segment_analog(Display* display, ace_time::ZonedDateTime* time, double, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
/** Binary clock that shows hour, minute and second in binary form: as horizontally stacked blocks. */
void binary(Display* display, ace_time::ZonedDateTime* time, double, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
/** Binary clock that shows seconds of day in binary form. Not very useful but fun to look at. */
void day_seconds_binary(Display* display, ace_time::ZonedDateTime* time, double, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);

static std::array<ClockFace, 7> clock_faces {
	&basic_digital,
	&basic_analog,
	&retro_analog,
	&modern_analog,
	&rotating_segment_analog,
	&binary,
	&day_seconds_binary,
};

}
