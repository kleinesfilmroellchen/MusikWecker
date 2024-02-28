// This should *really* be in the Arduino base libraries.
// Stolen with adjustments from https://github.com/RobTillaart/PrintString (MIT)
// (can you even have copyright on a file this simple?)

#pragma once

//
//    FILE: PrintString.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.2.4
// PURPOSE: Class that captures prints into a String
//    DATE: 2017-12-09
//     URL: https://github.com/RobTillaart/PrintString

#include <Arduino.h>
#include <Print.h>

class PrintString : public Print {
public:
    PrintString() = default;

    virtual size_t write(uint8_t c) override
    {
        buffer.concat(char(c));
        return 1;
    }

    virtual size_t write(uint8_t const* str, size_t length) override
    {
        buffer.concat((char*)str, length);
        return length;
    }

    size_t size()
    {
        return buffer.length();
    }

    void clear()
    {
        buffer = String();
    }

    String getString()
    {
        return buffer;
    }

private:
    String buffer;
};
