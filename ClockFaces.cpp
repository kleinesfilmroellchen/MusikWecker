#include "ClockFaces.h"
#include "Audio.h"
#include <stdint.h>

static uint8_t getCenter(uint8_t start, uint8_t size)
{
    return start + size / 2;
}

void basicDigitalCF(T_DISPLAY* display, ace_time::ZonedDateTime* time,
    uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
    char timeStr[6];
    sprintf(timeStr, "%02u:%02u", time->hour(), time->minute());
    yield();

    display->setFont(CLOCK_FONT);
    uint16_t textWidth = display->getUTF8Width(timeStr);
    yield();

    uint16_t textStart = textWidth > width ? x0 : (width - textWidth) / 2 + x0;
    display->drawUTF8(textStart, (height + CLOCK_FONT_HEIGHT) / 2 + y0, timeStr);
    yield();
}

void digitalWithSecondsCF(T_DISPLAY* display, ace_time::ZonedDateTime* time,
    uint8_t x0, uint8_t y0, uint8_t width,
    uint8_t height)
{

    char timeStr[6];
    sprintf(timeStr, "%02u:%02u", time->hour(), time->minute());
    yield();

    display->setFont(CLOCK_FONT);
    uint16_t textWidth = display->getUTF8Width(timeStr);
    uint16_t textStart = textWidth > width ? x0 : (width - textWidth) / 2 + x0;
    display->drawUTF8(textStart, (height + CLOCK_FONT_HEIGHT) / 2 + y0, timeStr);

    char secondStr[3];
    sprintf(secondStr, "%02u", time->second());
    yield();

    display->setFont(MAIN_FONT);
    display->drawUTF8(textStart + textWidth,
        (height + CLOCK_FONT_HEIGHT) / 2 + y0, secondStr);
    yield();
}

void basicAnalogCF(T_DISPLAY* display, ace_time::ZonedDateTime* time,
    uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
    const double hor = time->hour() + time->minute() / 60.0d,
                 minut = time->minute() + time->second() / 60.0d,
                 secnd = time->second();
    const uint16_t centerX = getCenter(x0, width),
                   centerY = getCenter(y0, height);

    // draw 12 line segments representing hours
    for (double i = 0; i < TWO_PI; i += PI_DIV6) {
        const double innerX = (cos(i - HALF_PI) * (ANALOGCF_SIZE / 2 - ANALOGCF_LINE_LENGTH)),
                     innerY = (sin(i - HALF_PI) * (ANALOGCF_SIZE / 2 - ANALOGCF_LINE_LENGTH)),
                     outerX = (cos(i - HALF_PI) * ANALOGCF_SIZE / 2),
                     outerY = (sin(i - HALF_PI) * ANALOGCF_SIZE / 2);

        yield();
        display->drawLine(innerX + centerX, innerY + centerY, outerX + centerX,
            outerY + centerY);
    }

    const double secondAngle = (secnd / 60.0d) * TWO_PI - HALF_PI,
                 minuteAngle = (minut / 60.0d) * TWO_PI - HALF_PI,
                 hourAngle = ((hor >= 12 ? hor - 12 : hor) / 12.0d) * TWO_PI - HALF_PI;
    const double secOuterX = cos(secondAngle) * (ANALOGCF_SIZE / 2 - ANALOGCF_LINE_LENGTH - 2),
                 secOuterY = sin(secondAngle) * (ANALOGCF_SIZE / 2 - ANALOGCF_LINE_LENGTH - 2);

    display->drawLine(centerX, centerY, centerX + secOuterX, centerY + secOuterY);

    yield();
    display->drawLine(centerX, centerY,
        cos(minuteAngle) * ANALOGCF_MINUTE_LENGTH + centerX,
        sin(minuteAngle) * ANALOGCF_MINUTE_LENGTH + centerY);

    yield();
    display->drawLine(centerX, centerY,
        cos(hourAngle) * ANALOGCF_HOUR_LENGTH + centerX,
        sin(hourAngle) * ANALOGCF_HOUR_LENGTH + centerY);
}

void modernAnalogCF(T_DISPLAY* display, ace_time::ZonedDateTime* time,
    uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
    const double hor = time->hour() + time->minute() / 60.0d,
                 minut = time->minute() + time->second() / 60.0d,
                 secnd = time->second();
    const uint16_t centerX = getCenter(x0, width),
                   centerY = getCenter(y0, height);

    yield();
    display->drawDisc(centerX, centerY, ANALOGCF_LINE_LENGTH);

    // draw 12 line segments representing hours
    for (double i = 0; i < TWO_PI; i += PI_DIV6) {
        const double innerX = (cos(i - HALF_PI) * (ANALOGCF_SIZE / 2 - ANALOGCF_LINE_LENGTH)),
                     innerY = (sin(i - HALF_PI) * (ANALOGCF_SIZE / 2 - ANALOGCF_LINE_LENGTH)),
                     outerX = (cos(i - HALF_PI) * ANALOGCF_SIZE / 2),
                     outerY = (sin(i - HALF_PI) * ANALOGCF_SIZE / 2);
        yield();
        display->drawLine(innerX + centerX, innerY + centerY, outerX + centerX,
            outerY + centerY);
    }

    const double secondAngle = (secnd / 60.0d) * TWO_PI - HALF_PI,
                 minuteAngle = (minut / 60.0d) * TWO_PI - HALF_PI,
                 hourAngle = ((hor >= 12 ? hor - 12 : hor) / 12.0d) * TWO_PI - HALF_PI;

    display->drawLine(centerX, centerY,
        cos(minuteAngle) * ANALOGCF_MINUTE_LENGTH + centerX,
        sin(minuteAngle) * ANALOGCF_MINUTE_LENGTH + centerY);
    yield();
    display->drawLine(centerX, centerY,
        cos(hourAngle) * ANALOGCF_HOUR_LENGTH + centerX,
        sin(hourAngle) * ANALOGCF_HOUR_LENGTH + centerY);
}

void rotatingSegmentAnalogCF(T_DISPLAY* display, ace_time::ZonedDateTime* time,
    uint8_t x0, uint8_t y0, uint8_t width,
    uint8_t height)
{
    const double hor = time->hour() + time->minute() / 60.0d,
                 minut = time->minute() + time->second() / 60.0d,
                 secnd = time->second();
    const uint16_t centerX = getCenter(x0, width),
                   centerY = getCenter(y0, height);

    const double secondAngle = secnd * TWO_PI / 60.0d,
                 minuteAngle = minut * TWO_PI / 60.0d,
                 hourAngle = (hor >= 12 ? hor - 12 : hor) * TWO_PI / 12.0d;

    // the offsets need to be as smooth as possible, but not time-precise
    // make them dependent on internal milliseconds, which will only glitch out
    // every 50 days or so
    const double secondOffset = fmod(millis() / 1000.0d / VRAND0, TWO_PI);
    const double minuteOffset = fmod(-millis() / 1400.0d / VRAND1, TWO_PI);
    const double hourOffset = fmod(millis() / 2600.0d / VRAND2, TWO_PI);

    yield();
    drawArc(display, centerX, centerY, height / 2 - LINESEP * 2, secondOffset,
        secondOffset + secondAngle);
    yield();
    drawArc(display, centerX, centerY, height / 2 - LINESEP * 4, minuteOffset,
        minuteOffset + minuteAngle);
    yield();
    drawArc(display, centerX, centerY, height / 2 - LINESEP * 8, hourOffset,
        hourOffset + hourAngle);
}

void binaryCF(T_DISPLAY* display, ace_time::ZonedDateTime* time, uint8_t x0,
    uint8_t y0, uint8_t width, uint8_t height)
{
    const uint8_t hor = time->hour(), minut = time->minute(),
                  secnd = time->second();
    // vertical position of each row
    const uint16_t hourpos = (height - BINARYCF_BOX_SIZE) / 2 - (BINARYCF_BOX_SIZE + BINARYCF_BOX_SPACING) + y0,
                   minutepos = (height - BINARYCF_BOX_SIZE) / 2 + y0,
                   secondpos = (height - BINARYCF_BOX_SIZE) / 2 + (BINARYCF_BOX_SIZE + BINARYCF_BOX_SPACING) + y0;
    // maximum of 6 bits for minute/second, 5 bits for hour, go with one
    // 6-bit-loop
    for (uint8_t bit = 0; bit < 6; ++bit) {
        yield();
        if (hor & (1 << bit)) {
            display->drawBox(bit * (BINARYCF_BOX_SIZE + BINARYCF_BOX_SPACING) + BINARYCF_BOX_SPACING + x0,
                hourpos, BINARYCF_BOX_SIZE, BINARYCF_BOX_SIZE);
        }
        if (minut & (1 << bit)) {
            display->drawBox(bit * (BINARYCF_BOX_SIZE + BINARYCF_BOX_SPACING) + BINARYCF_BOX_SPACING + x0,
                minutepos, BINARYCF_BOX_SIZE, BINARYCF_BOX_SIZE);
        }
        if (secnd & (1 << bit)) {
            display->drawBox(bit * (BINARYCF_BOX_SIZE + BINARYCF_BOX_SPACING) + BINARYCF_BOX_SPACING + x0,
                secondpos, BINARYCF_BOX_SIZE, BINARYCF_BOX_SIZE);
        }
    }
}

void fullDayBinaryCF(T_DISPLAY* display, ace_time::ZonedDateTime* time,
    uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
    const uint32_t secnd = time->second() + time->minute() * 60 + time->hour() * 60 * 24;

    uint16_t ypos = (height - BINARYCF_BOX_SIZE) / 2;
    // maximum of 17 bits for second of day
    for (uint16_t xpos = BINARYCF_BOX_SPACING, bit = 0; bit < 17;
         ++bit, xpos += BINARYCF_BOX_SPACING + BINARYCF_BOX_SIZE) {
        yield();
        // "line wrap" if boxes would overshoot the screen width
        if (xpos + BINARYCF_BOX_SIZE > width) {
            xpos = BINARYCF_BOX_SPACING;
            ypos += BINARYCF_BOX_SIZE + BINARYCF_BOX_SPACING;
        }
        if (secnd & (1 << bit)) {
            display->drawBox(xpos + x0, ypos + y0, BINARYCF_BOX_SIZE,
                BINARYCF_BOX_SIZE);
        }
    }
}