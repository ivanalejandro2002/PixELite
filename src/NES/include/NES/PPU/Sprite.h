#ifndef NES_PPU_SPRITE_H
#define NES_PPU_SPRITE_H

#include <cstdint>

namespace NES
{
    namespace PPU
    {
        struct Sprite
        {
            uint8_t y = 0;
            uint8_t tileIndex = 0;
            uint8_t attributes = 0;
            uint8_t x = 0;
            bool isSpriteZero = false;

            uint8_t palette() const
            {
                return attributes & 0x03;
            }

            bool behindBackground() const
            {
                return (attributes & 0x20) != 0;
            }

            bool flipHorizontal() const
            {
                return (attributes & 0x40) != 0;
            }

            bool flipVertical() const
            {
                return (attributes & 0x80) != 0;
            }

            bool isOnScanline(uint8_t currentScanline, uint8_t spriteHeight) const
            {
                return getRowInSprite(currentScanline, spriteHeight) != -1;
            }

            int getRowInSprite(uint8_t currentScanline, uint8_t spriteHeight) const
            {
                int startLine = static_cast<int>(y) + 1;

                int row = static_cast<int>(currentScanline) - startLine;

                if (row < 0 || row >= spriteHeight)
                {
                    return -1;
                }

                if (attributes & 0x80)
                {
                    row = (spriteHeight - 1) - row;
                }

                return row;
            }
        };
    }
}

#endif