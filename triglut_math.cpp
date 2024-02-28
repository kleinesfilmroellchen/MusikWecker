#include "triglut_math.h"

float lut_fastsin(double angle)
{
    // find the closest value to the given angle in the lut and compute the index
    uint16_t index = (uint16_t)round(fmod(angle, TWO_PI) * 209 / TWO_PI);
    // double is 64 bits = 8 bytes = two double words = quad word
    // big endian??
    float val = pgm_read_float(&sin_lut[index]);
    return val;
}

float lut_fastcos(double angle)
{
    uint16_t index = (uint16_t)round(fmod(angle, TWO_PI) * 209 / TWO_PI);
    float val = pgm_read_float(&cos_lut[index]);
    return val;
}
