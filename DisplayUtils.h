/** Display utilities header. */

#pragma once

#include "defs.h"
#include "triglut_math.h"
#include <U8g2lib.h>

/**
  Draws an arc, i.e. a circle section, of thickness 1 with specified radius and start and end angles.
*/
inline void drawArc(T_DISPLAY* disp, uint16_t x0, uint16_t y0, uint16_t radius, double startAngle, double endAngle);

/**
   Calculates the pixel position of the specified line (index) depending on font settings.
*/
inline uint16_t linepos(uint8_t line);

inline void drawArc(T_DISPLAY* disp, uint16_t x0, uint16_t y0, uint16_t radius, double startAngle, double endAngle)
{
    if (startAngle > TWO_PI && endAngle > TWO_PI) {
        startAngle = fmod(startAngle, TWO_PI);
        endAngle = fmod(endAngle, TWO_PI);
    }
    // switch angles if necessary
    double smallerAngle = min(startAngle, endAngle), largerAngle = max(startAngle, endAngle);
    //  Serial.printf("%3f -> %3f", degrees(smallerAngle), degrees(largerAngle)); debug_print();
    // constrain to 0 -> 2π

    // i tried Bresenham for four hours. now i'm giving up.
    int16_t x, y;
    // the increment is chosen so that on about 64 pixel diameter arcs, no gaps appear
    // starting the angle at a boundary aligned to the iteration step stops repeated draws from flickering
    for (double a = round(smallerAngle / ARC_ITERATION_STEP) * ARC_ITERATION_STEP; a < largerAngle; a += ARC_ITERATION_STEP) {
        //    Serial.printf("%.4f cos %.4f, %.4f sin %.4f\n", a, lut_fastcos(a), a, lut_fastsin(a));
        x = round(lut_fastcos(a) * radius);
        y = round(lut_fastsin(a) * radius);
        //    Serial.printf("%d, %d\n", x, y);
        disp->drawPixel(x + x0, y + y0);
    }
}

// function definitions here b/c always inline switch needs the body to be available in the same translation unit

inline uint16_t linepos(uint8_t line)
{
    return line * LINE_HEIGHT;
}

/**
   Draw a string onto the display at specified line.
*/
inline void drawString(T_DISPLAY* disp, char const* c_text, uint8_t line)
{
    String string = c_text;
    std::vector<String> lines;
    auto firstNewline = string.indexOf('\n');
    if (firstNewline < 0) {
        lines.push_back(string);
    } else {
        do {
            auto start = string.substring(0, firstNewline);
            string = string.substring(firstNewline + 1);
            lines.push_back(start);
            firstNewline = string.indexOf('\n');
        } while (firstNewline >= 0 && !string.isEmpty());
        if (!string.isEmpty())
            lines.push_back(string);
    }

    size_t current_line = line;
    for (auto const& line : lines) {
        disp->drawUTF8(LEFT_TEXT_MARGIN, MAIN_FONT_SIZE + linepos(current_line) + 1, line.c_str());
        current_line++;
    }
}
