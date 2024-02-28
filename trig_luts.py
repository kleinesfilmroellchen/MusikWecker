from math import pi, sin, cos, floor
from numpy import arange

# determines how much space the luts will take.
# consider this carefully, even though the luts are in flash
# with default of 0.03, only 210 elements per lut are used, which are 2 * 836 bytes (about 1.7KB or 0.5% of flash memory)
increment = 0.03
lut_size = int(floor(pi * 2 / 0.03)) + 1
print(lut_size)

sin_lut = []
cos_lut = []

code_template = f"""
#include <Arduino.h>

#pragma once

const float sin_lut[{lut_size}] PROGMEM = {{}};
const float cos_lut[{lut_size}] PROGMEM = {{}};
// tan lut is not needed right now

"""
# sinlut_toformat = 'sin_lut[{}] = {};\n'
# coslut_toformat = 'cos_lut[{}] = {};\n'

for i in arange(0.0, pi * 2.0, increment):
    sin_lut.append(sin(i))
    cos_lut.append(cos(i))

print(sin_lut, len(sin_lut))
print(cos_lut, len(cos_lut))

with open("trig_luts.h", "w") as trigfile_c:
    full_sin_array = "{" + ", ".join(map(lambda f: str(f), sin_lut)) + "}"
    full_cos_array = "{" + ", ".join(map(lambda f: str(f), cos_lut)) + "}"
    trigfile_c.write(code_template.format(full_sin_array, full_cos_array))
    # for i, lutvals in enumerate(zip(sin_lut, cos_lut)):
    # 	sinval, cosval = lutvals
    # 	trigfile_c.write(sinlut_toformat.format(i, sinval))
    # 	trigfile_c.write(coslut_toformat.format(i, cosval))
