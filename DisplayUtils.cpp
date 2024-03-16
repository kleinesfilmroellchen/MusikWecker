#include "DisplayUtils.h"

void draw_rotated_xbm(Display* display, uint16_t x_start, uint16_t y_start,
	double angle, uint16_t w, uint16_t h,
	uint8_t const* bitmap)
{
	auto* current_bitmap_pointer = bitmap;
	auto y = y_start;
	uint8_t mask = 1;

	auto remaining_height = h;
	while (remaining_height > 0) {
		auto x = x_start;
		auto len = w;

		while (len > 0) {
			if ((*current_bitmap_pointer & mask) >= 0) {
				display->drawPixel(x, y);
			}
			x++;
			mask <<= 1;
			if (mask == 0) {
				mask = 1;
				current_bitmap_pointer++;
			}
			len--;
		}

		y++;
		remaining_height--;
	}
}

void draw_arc(Display* display, uint16_t x0, uint16_t y0, uint16_t radius, double startAngle, double endAngle)
{
	// constrain to -π -> π
	startAngle = fmod(startAngle, TWO_PI);
	startAngle -= PI;
	endAngle = fmod(endAngle, TWO_PI);
	endAngle -= PI;
	if (startAngle == endAngle)
		return;
	// make sure that end > start so we don't have to deal with too much modular arithmetic later
	if (endAngle < startAngle)
		endAngle += TWO_PI;

	// FIXME: There's probably a faster way of checking whether these X and Y coordinates are in our required range
	// than throwing them through trig functions.
	auto draw = [&](auto x, auto y) {
		auto angle = atan2f(x, y);
		// if angle is below the start, it may actually be in range,
		// but since the end angle was raised, we can't detect that with the simple range check.
		if (angle < startAngle)
			angle += TWO_PI;

		if (angle >= startAngle && angle <= endAngle)
			display->drawPixel(x0 + x, y0 + y);
	};

	// Bresenham for circles
	auto f = 1 - radius;
	auto ddF_x = 0;
	auto ddF_y = -2 * radius;
	auto x = 0;
	auto y = radius;

	draw(0, +radius);
	draw(0, -radius);
	draw(+radius, 0);
	draw(-radius, 0);

	// make sure we draw at least one pixel for small angles if we didn't return above.
	// (don't use the draw() helper since the range check would likely fail)
	display->drawPixel(x0 + round(cos(-startAngle + HALF_PI) * radius), y0 + round(sin(-startAngle + HALF_PI) * radius));
	display->drawPixel(x0 + round(cos(-endAngle + HALF_PI) * radius), y0 + round(sin(-endAngle + HALF_PI) * radius));

	while (x < y) {
		if (f >= 0) {
			y -= 1;
			ddF_y += 2;
			f += ddF_y;
		}
		x += 1;
		ddF_x += 2;
		f += ddF_x + 1;

		draw(+x, +y);
		draw(-x, +y);
		draw(+x, -y);
		draw(-x, -y);
		draw(+y, +x);
		draw(-y, +x);
		draw(+y, -x);
		draw(-y, -x);
	}
}

void draw_string(Display* display, char const* c_text, uint8_t line)
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
		yield();
		display->drawUTF8(LEFT_TEXT_MARGIN, MAIN_FONT_SIZE + position_of_line(current_line) + 1, line.c_str());
		current_line++;
	}
}
