#include "SRLV.h"

#ifndef pgm_read_byte
#define pgm_read_byte(x) (*(x))
#endif
#ifndef yield
#define yield()
#endif

namespace SRLV {

constexpr uint8_t full_byte_marker = 0x80;
constexpr uint8_t rle_length_limit = 0x7f;

std::vector<uint8_t> decompress(Span<uint8_t> data, size_t pixel_count)
{
	// 0 = black, 1 = white (as usual with XBM)
	uint8_t current_color = 0;
	// bit position within the byte that is going to be written next
	uint8_t current_bit = 1;
	uint8_t current_byte = 0;
	std::vector<uint8_t> output;
	output.reserve(pixel_count / 8);
	size_t written_pixels = 0;

	auto next_pixel = [&] {
		++written_pixels;
		current_bit <<= 1;
		// bit was shifted out, restart the next byte
		if (current_bit == 0) {
			output.push_back(current_byte);
			current_byte = 0;
			current_bit = 1;
		}
	};

	auto handle_byte = [&](uint8_t input_byte) {
		size_t i = 0;
		for (i = 0; i < input_byte; ++i) {
			yield();
			// optimization: write entire bytes as long as possible
			if (current_bit == 1 && i + 8 <= input_byte) {
				i += 7;
				written_pixels += 8;
				output.push_back(current_color == 1 ? 0xff : 0);
				continue;
			}
			if (current_color == 1)
				current_byte |= current_bit;
			next_pixel();
		}
		current_color = 1 - current_color;
	};

	for (size_t i = 0; i < data.size(); ++i) {
		yield();
		auto input_byte = pgm_read_byte(data.offset_pointer(i));

		if ((input_byte & full_byte_marker) > 0) {
			handle_byte(input_byte & rle_length_limit);
		} else {
			handle_byte(input_byte >> 4);
			handle_byte(input_byte & 0xf);
		}
	}

	// continue with the last color until the end
	current_color = 1 - current_color;
	while (written_pixels < pixel_count)
		next_pixel();

	return output;
}

}
