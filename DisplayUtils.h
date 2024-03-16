/** Display utilities header. */

#pragma once

#include "Audio.h"
#include "Definitions.h"
#include "triglut_math.h"
#include <U8g2lib.h>
#include <array>

/**
   Calculates the pixel position of the specified line (index) depending on font settings.
*/
inline uint16_t position_of_line(uint8_t line)
{
	return line * LINE_HEIGHT;
}

/**
  Draws an arc, i.e. a circle section, of thickness 1 with specified radius and start and end angles.
*/
void draw_arc(Display* display, uint16_t x0, uint16_t y0, uint16_t radius, double start_angle, double end_angle);

/**
   Draw a string onto the display at specified line.
*/
void draw_string(Display* display, char const* c_text, uint8_t line);

void draw_rotated_xbm(Display* display, uint16_t x, uint16_t y, double angle, uint16_t w, uint16_t h, uint8_t const* bitmap);
