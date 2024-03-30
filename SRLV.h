#pragma once

#include <cstdint>
#include <stdint.h>
#include <vector>

#include "Span.h"

namespace SRLV {
std::vector<uint8_t> decompress(Span<uint8_t> data, size_t pixel_count, Span<uint8_t> previous_frame);
}
