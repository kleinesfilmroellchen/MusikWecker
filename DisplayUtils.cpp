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

void draw_arc(Display* display, uint16_t x0, uint16_t y0, uint16_t radius, double start_angle, double end_angle)
{
	// constrain to -π -> π
	start_angle = fmod(start_angle, TWO_PI);
	start_angle -= PI;
	end_angle = fmod(end_angle, TWO_PI);
	end_angle -= PI;
	if (start_angle == end_angle)
		return;
	// make sure that end > start so we don't have to deal with too much modular arithmetic later
	if (end_angle < start_angle)
		end_angle += TWO_PI;

	// FIXME: There's probably a faster way of checking whether these X and Y coordinates are in our required range
	// than throwing them through trig functions.
	auto draw = [&](auto x, auto y) {
		auto angle = atan2f(x, y);
		// if angle is below the start, it may actually be in range,
		// but since the end angle was raised, we can't detect that with the simple range check.
		if (angle < start_angle)
			angle += TWO_PI;

		if (angle >= start_angle && angle <= end_angle)
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
	display->drawPixel(x0 + round(cos(-start_angle + HALF_PI) * radius), y0 + round(sin(-start_angle + HALF_PI) * radius));
	display->drawPixel(x0 + round(cos(-end_angle + HALF_PI) * radius), y0 + round(sin(-end_angle + HALF_PI) * radius));

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
		yield();
	}
}

void draw_string(Display* display, char const* c_text, uint8_t line)
{
	String string = c_text;
	std::vector<String> lines;
	auto first_newline = string.indexOf('\n');
	if (first_newline < 0) {
		lines.push_back(string);
	} else {
		do {
			auto start = string.substring(0, first_newline);
			string = string.substring(first_newline + 1);
			lines.push_back(start);
			first_newline = string.indexOf('\n');
		} while (first_newline >= 0 && !string.isEmpty());
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

int8_t point_index(double angle)
{
	return static_cast<int8_t>(round(angle / QUARTER_PI) + 8) % 8;
}

Point adjacent_point_for(double angle)
{
	return adjacency[point_index(angle)];
}

Point previous_adjacent_point_for(double angle)
{
	return adjacency[(point_index(angle) + 9) % 8];
}

void draw_stroked_line(Display* display, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, double angle, uint8_t stroke_width)
{
	const auto stroke_direction = adjacent_point_for(angle);
	const auto previous_stroke_direction = previous_adjacent_point_for(angle);

	for (size_t i = 1; i <= stroke_width; ++i) {
		const uint8_t offset = i / 2;
		const int8_t direction = i % 2 == 0 ? 1 : -1;

		display->drawLine(x0 + offset * direction * stroke_direction.x,
			y0 + offset * direction * stroke_direction.y,
			x1 + offset * direction * stroke_direction.x,
			y1 + offset * direction * stroke_direction.y);
		display->drawLine(x0 + offset * direction * previous_stroke_direction.x,
			y0 + offset * direction * previous_stroke_direction.y,
			x1 + offset * direction * previous_stroke_direction.x,
			y1 + offset * direction * previous_stroke_direction.y);
	}
}
