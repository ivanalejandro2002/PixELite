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
                uint16_t startLine = (y + 1) & 0xFF;

                // Resta con signo normal: si currentScanline es menor que startLine, dará negativo.
                int row = static_cast<int>(currentScanline) - static_cast<int>(startLine);

                // Si está fuera del rango del sprite, simplemente devuelve -1 (no se dibuja)
                if (row < 0 || row >= spriteHeight)
                {
                    return -1;
                }

                // Si el sprite tiene volteo vertical (Bit 7 de attributes), se invierte la fila
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