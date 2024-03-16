import csv
import regex

header = """
#pragma once
#include <Arduino.h>
#include "Span.h"
"""

coord_re = regex.compile(
    r"(?:[\+\-][0-9]{4,6})(([\+\-])([0-9]{2,3}?)([0-9]{2})([0-9]{2})?)"
)

zoneset = set()


def main():
    with open("zone1970.tab", newline="") as zonedata:
        rdr = csv.reader(zonedata, delimiter="\t", quoting=csv.QUOTE_NONE)
        for _, location, name, *_ in rdr:
            _, eastwest, degrees, minutes, seconds = coord_re.fullmatch(
                location
            ).groups()
            fract_degrees = (1 if eastwest == "+" else -1) * (
                int(degrees)
                + int(minutes) / 60.0
                + (int(seconds) / 3600.0 if seconds is not None else 0)
            )
            print(fract_degrees, degrees, minutes, seconds, name)
            zoneset.add((fract_degrees, name))

    zonelist = sorted(zoneset, key=lambda elt: elt[0])
    for location, name in zonelist:
        print("{:+09.4f} : {:s}".format(location, name))

    with open("zonelist.h", "w") as output:
        print(header, file=output)
        i = 0
        for _, name in zonelist:
            print(f'const char tzlist_{i}[] PROGMEM = "{name}";', file=output)
            i = i + 1
        print(f"constexpr size_t ZONE_COUNT = {i};", file=output)
        print(
            f'static char const* tzlist_array[] PROGMEM = {{ {", ".join(("tzlist_" + str(i) for i in range(i)))} }};',
            file=output,
        )
        print("const Span<char const*> tzlist = { tzlist_array };", file=output)


if __name__ == "__main__":
    main()
