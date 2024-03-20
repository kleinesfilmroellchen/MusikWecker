#include "SRLV.h"
#include <bit>

#ifndef pgm_read_byte
#define pgm_read_byte(x) (*(x))
#endif
#ifndef yield
#define yield()
#endif

#include <iostream>

namespace SRLV {

constexpr uint8_t full_byte_marker = 0x80;
constexpr uint8_t rle_length_limit = 0x7f;
constexpr uint8_t row_size = 80 / 8;

enum class CompressionMode : uint8_t {
	Nibble = 0,
	NibbleDelta = 1,
	NibbleSnake = 2,
	Pokemon = 3,
	PokemonDelta = 4,
	PokemonSnake = 5,
};

static std::vector<uint8_t> decompress_nibble(Span<uint8_t> data, size_t pixel_count)
{
	// 0 = black, 1 = white (as usual with XBM)
	uint8_t current_color = 0;
	// bit position within the byte that is going to be written next
	uint8_t current_bit = 1;
	uint8_t current_byte = 0;
	std::vector<uint8_t> output;
	auto byte_count = pixel_count / 8 + 1;
	output.reserve(byte_count);

	auto next_pixel = [&] {
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
	while (current_bit != 1) {
		if (current_color == 1)
			current_byte |= current_bit;
		next_pixel();
	}

	output.resize(byte_count, current_color == 1 ? 0xff : 0);
	return output;
}

static std::vector<uint8_t> decompress_pokemon(Span<uint8_t> data, size_t pixel_count)
{
	uint8_t current_bit = 1;
	uint8_t current_byte = 0;
	std::vector<uint8_t> output;
	auto byte_count = pixel_count / 8 + 1;
	output.reserve(byte_count);

	auto next_pixel = [&] {
		current_bit <<= 1;
		// bit was shifted out, restart the next byte
		if (current_bit == 0) {
			output.push_back(current_byte);
			current_byte = 0;
			current_bit = 1;
		}
	};

	for (size_t byte_index = 0; byte_index < data.size(); ++byte_index) {
		uint8_t byte = pgm_read_byte(data.offset_pointer(byte_index));
		yield();
		uint8_t data = byte & rle_length_limit;
		if ((byte & full_byte_marker) > 0) {
			uint8_t run_length = data + 1;
			uint8_t i = 0;
			for (; (current_bit != 1) && i < run_length; ++i)
				next_pixel();
			for (; i + 8 < run_length; i += 8)
				output.push_back(0);
			for (; i < run_length; ++i)
				next_pixel();
		} else {
			for (uint8_t i = 0; i < 7; ++i) {
				// shift in remaining bits directly
				if (current_bit == 1) {
					current_byte = data >> i;
					current_bit = 1 << (7 - i);
					break;
				}
				uint8_t pixel_value = data & (1 << i);
				if (pixel_value > 0)
					current_byte |= current_bit;
				next_pixel();
			}
		}
	}

	while (current_bit != 1)
		next_pixel();

	output.resize(byte_count, 0);

	uint8_t previous_pixel = 0;
	for (size_t i = 0; i < output.size(); ++i) {
		uint8_t byte = output[i];
		uint8_t new_byte = 0;
		for (auto bit = 0; bit <= 7; ++bit) {
			uint8_t delta = (byte & (1 << bit)) > 0;
			uint8_t pixel_value = delta ^ previous_pixel;
			previous_pixel = pixel_value;
			new_byte |= (pixel_value << bit);
		}
		output[i] = new_byte;
	}

	return output;
}

static void decompress_delta(Span<uint8_t> current, Span<uint8_t> previous)
{
	for (size_t i = 0; i < current.size(); ++i) {
		current[i] ^= previous[i];
	}
}

constexpr uint8_t bitswap(uint8_t x)
{
	x = ((x & 0x55) << 1) | ((x & 0xAA) >> 1);
	x = ((x & 0x33) << 2) | ((x & 0xCC) >> 2);
	x = ((x & 0x0F) << 4) | ((x & 0xF0) >> 4);
	return x;
}

static void reorder_snake(Span<uint8_t> frame)
{
	for (size_t row = 0;; ++row) {
		if (row * row_size >= frame.size())
			break;

		// only switch odd rows
		if (row % 2 == 0)
			continue;

		for (size_t i = 0; i < row_size / 2; ++i) {
			auto front_index = row * row_size + i;
			auto back_index = row * row_size + (row_size - 1 - i);
			auto front = frame[front_index];
			auto back = frame[back_index];
			frame[front_index] = bitswap(back);
			frame[back_index] = bitswap(front);
		}
	}
}

std::vector<uint8_t> decompress(Span<uint8_t> data, size_t pixel_count, Span<uint8_t> previous_frame)
{
	auto mode = static_cast<CompressionMode>(data[0]);
	data = data.slice(1);
	std::vector<uint8_t> output;
	if (mode == CompressionMode::Nibble || mode == CompressionMode::NibbleDelta || mode == CompressionMode::NibbleSnake) {
		output = decompress_nibble(data, pixel_count);
	} else {
		output = decompress_pokemon(data, pixel_count);
	}

	if (mode == CompressionMode::NibbleDelta || mode == CompressionMode::PokemonDelta) {
		decompress_delta(output, previous_frame);
	} else if (mode == CompressionMode::NibbleSnake || mode == CompressionMode::PokemonSnake) {
		reorder_snake(output);
	}

	return output;
}

}
