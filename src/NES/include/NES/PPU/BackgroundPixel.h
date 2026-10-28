#pragma once

#include <cstdint>

namespace NES::PPU
{
    struct BackgroundPixel
    {
        uint8_t color = 0;
        bool transparent = true;
    };
}