#include <SRLV.h>
#include <Span.h>
#include <bit>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdint.h>

#define PROGMEM

#include "../bad_apple.h"

constexpr int IMAGE_WIDTH = 80;
constexpr int IMAGE_HEIGHT = 64;
constexpr int PIXEL_COUNT = IMAGE_WIDTH * IMAGE_HEIGHT;
constexpr size_t ROW_SIZE = IMAGE_WIDTH / 8;

void print_xbm(Span<uint8_t> xbm_data)
{
	auto x = 0;
	auto y = 0;
	for (auto const data : xbm_data) {
		std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(data) << " ";
		x += 8;
		if (x >= IMAGE_WIDTH) {
			x = 0;
			y += 1;
			std::cout << std::endl;
		}
	}
	std::cout << std::endl
			  << y << std::endl;
	std::cout << xbm_data.size() << std::endl;
}

int main()
{
	auto image = SRLV::decompress({ frame_1910, sizeof(frame_1910) }, IMAGE_WIDTH * IMAGE_HEIGHT, { });
	print_xbm(image);
}
