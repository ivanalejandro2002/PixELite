#pragma once

#include <cstdint>

namespace NES::PPU
{
    struct SpritePixel
    {
        uint8_t color = 0;
        bool transparent = true;
    };
}