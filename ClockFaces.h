/**Clock face functions header*/

#pragma once

#include "DisplayUtils.h"
#include "defs.h"
#include "triglut_math.h"
#include <AceTime.h>
#include <U8g2lib.h>

// size of the analog clock face
constexpr uint16_t ANALOGCF_SIZE = 56;
// length of the line segments that represent the hour markings
constexpr uint16_t ANALOGCF_LINE_LENGTH = 4;
// length of the hour hand
constexpr uint16_t ANALOGCF_HOUR_LENGTH = 8;
// length of the minute hand
constexpr uint16_t ANALOGCF_MINUTE_LENGTH = 15;
// size of the boxes in binary clocks
constexpr uint16_t BINARYCF_BOX_SIZE = 8;
// spacing between the boxes in binary clocks
constexpr uint16_t BINARYCF_BOX_SPACING = 2;
// computed maximum number of boxes in a row
constexpr uint8_t BINARYCF_MAX_ROWBOXES = static_cast<uint8_t>(SCREEN_WIDTH / (BINARYCF_BOX_SIZE + BINARYCF_BOX_SPACING));

// typedef the clock face function pointer type
using ClockFace = void (*)(T_DISPLAY*, ace_time::ZonedDateTime*, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);

/** Basic digital 24-hour clock. */
void basicDigitalCF(T_DISPLAY* display, ace_time::ZonedDateTime* time, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
/** Basic analog clock without numbers. */
void basicAnalogCF(T_DISPLAY* display, ace_time::ZonedDateTime* time, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
/** Analog clock with rotating segments for each time division. */
void rotatingSegmentAnalogCF(T_DISPLAY* display, ace_time::ZonedDateTime* time, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
/** Digital 24-hour clock with number second indicator. */
void digitalWithSecondsCF(T_DISPLAY* display, ace_time::ZonedDateTime* time, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
/** Binary clock that shows hour, minute and second in binary form: as horizontally stacked blocks. */
void binaryCF(T_DISPLAY* display, ace_time::ZonedDateTime* time, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
/** Binary clock that shows seconds of day in binary form. Not very useful but fun to look at. */
void fullDayBinaryCF(T_DISPLAY* display, ace_time::ZonedDateTime* time, uint8_t x0, uint8_t y0, uint8_t width, uint8_t height);
