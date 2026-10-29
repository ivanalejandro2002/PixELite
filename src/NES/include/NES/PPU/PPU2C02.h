#pragma once

#include <NES/Bus/Bus.h>
#include <NES/PPU/CIRAM.h>
#include <NES/PPU/PaletteRAM.h>
#include <NES/Cartridge/CHRROM.h>
#include <Graphics/FrameBuffer.h>

#include <array>
#include <cstdint>
#include <vector>
#include <NES/PPU/Sprite.h>
#include <NES/PPU/SpritePixel.h>
#include <NES/PPU/BackgroundPixel.h>

namespace NES
{
        class PPU2C02
        {
            public:
                PPU2C02(
                    const std::vector<uint8_t> &chrData,
                    CIRAM::Mirroring mirroring
                );

                void connectMainBus(Bus *b);

                void oamDma(uint8_t pageValue);

                void evaluateSprites(uint8_t scanline);
                void evaluateSingleSprite(uint8_t index, int targetScanline);

                uint8_t getSpritePixel(
                    const NES::PPU::Sprite& sprite,
                    uint8_t x,
                    uint8_t row,
                    int spriteIndex
                );

                uint8_t getSpritePixel(
                    const NES::PPU::Sprite& sprite,
                    uint8_t x,
                    uint8_t row
                );

                NES::PPU::SpritePixel getSpritePixelColor(
                    const NES::PPU::Sprite& sprite,
                    uint8_t x,
                    uint8_t row
                );

                uint8_t getSpritePaletteColor(
                    const NES::PPU::Sprite& sprite,
                    uint8_t pixelValue
                );

                NES::PPU::BackgroundPixel getBackgroundPixel();

                const Graphics::FrameBuffer& getFramebuffer() const;
                void debugGenerateTestPattern();
                void debugGenerateBackground();

                void clock();

                uint8_t readVRAM(uint16_t addr);
                void writeVRAM(uint16_t addr, uint8_t value);

                uint8_t readRegister(uint16_t addr);
                void writeRegister(uint16_t addr, uint8_t value);

                uint16_t getVRAMAddress() const;

                void writePPUCTRL(uint8_t value);
                void writePPUMASK(uint8_t value);

                uint8_t readPPUSTATUS();

                void writeOAMADDR(uint8_t value);
                uint8_t readOAMDATA();
                void writeOAMDATA(uint8_t value);

                void writePPUSCROLL(uint8_t value);
                void writePPUADDR(uint8_t value);

                uint8_t readPPUDATA();
                void writePPUDATA(uint8_t value);

                NES::PPU::Sprite getSprite(uint8_t index) const;
                Bus* getMainBus() const;

                void reset();

                bool isFrameComplete() const;
                void clearFrameComplete();

                uint8_t debugPPUCTRL() const;
                uint8_t debugPPUMASK() const;
                uint8_t debugPPUSTATUS() const;
                uint8_t debugOAMADDR() const;
                uint8_t debugOAMDATA() const;
                uint8_t debugPPUSCROLL() const;
                uint8_t debugPPUADDR() const;
                uint8_t debugPPUDATA() const;

                uint16_t debugV() const;
                uint16_t debugT() const;
                uint8_t debugX() const;
                bool debugW() const;

                uint16_t debugCycle() const;
                int16_t debugScanline() const;

                uint8_t debugScanlineSpriteCount() const;

                const NES::PPU::Sprite& debugScanlineSprite(uint8_t index) const;

            private:
                Bus bus;
                Bus *mainBus = nullptr;

                CHRROM chr;
                CIRAM ciram;
                PaletteRAM palette;

                uint8_t PPUCTRL = 0;
                uint8_t PPUMASK = 0;
                uint8_t PPUSTATUS = 0;

                uint8_t OAMADDR = 0;

                std::array<uint8_t, 256> OAM{};

                Graphics::FrameBuffer frameBuffer;

                uint16_t v, t;

                uint8_t fineX;

                bool writeToggle;

                uint8_t readBuffer;

                uint8_t ioLatch;

                uint16_t bgPatternShiftLow = 0;
                uint16_t bgPatternShiftHigh = 0;

                uint16_t bgAttributeShiftLow = 0;
                uint16_t bgAttributeShiftHigh = 0;

                uint8_t nextTileID = 0;
                uint8_t nextTileAttribute = 0;
                uint8_t nextTileLow = 0;
                uint8_t nextTileHigh = 0;

                
                
                int16_t scanline = 0;
                bool frameComplete = false;
                static constexpr uint8_t MAX_SPRITES_PER_SCANLINE = 8;

                NES::PPU::Sprite currentScanlineSprites[8];
                NES::PPU::Sprite nextScanlineSprites[8];
                std::array<uint8_t, 8> spritePatternLowBytes{};
                std::array<uint8_t, 8> spritePatternHighBytes{};
                std::array<uint8_t, 8> nextSpritePatternLowBytes{};
                std::array<uint8_t, 8> nextSpritePatternHighBytes{};
                uint8_t currentScanlineSpriteCount = 0;
                uint8_t nextScanlineSpriteCount = 0;
                uint8_t spriteEvaluationIndex = 0;
                bool oddCycle = false;
                bool oddFrame = false;
                uint8_t oamM = 0;

                uint16_t cycle = 0;

                void incrementCoarseX();
                void incrementY();
                void transferAddressX();
                void transferAddressY();

                void loadBackgroundShifters();
                void updateBackgroundShifters();
                void renderBackgroundPixel();
                void renderPixel();

                void backgroundFetch();

                std::array<uint8_t, 3> applyColorEmphasis(
                    const std::array<uint8_t, 3>& color
                ) const;

                struct EvaluatedSpritePixel {
                    uint8_t colorIndex = 0;
                    bool transparent = true;
                    bool behindBackground = false;
                    bool isSpriteZero = false; // Útil para el Sprite 0 Hit
                };

                EvaluatedSpritePixel getTopSpritePixel(int x, int scanline)
                {
                    const bool showSprites = (PPUMASK & 0x10) != 0;
                    const bool showLeftSprites = (PPUMASK & 0x04) != 0;

                    if (!showSprites) return {0, true, false, false};
                    if (x < 8 && !showLeftSprites) return {0, true, false, false};

                    const bool is8x16 = (PPUCTRL & 0x20) != 0;
                    const uint8_t spriteHeight = is8x16 ? 16 : 8;

                    for (uint8_t i = 0; i < currentScanlineSpriteCount; ++i)
                    {
                        const auto& sprite = currentScanlineSprites[i];
                        
                        int spriteX = sprite.x;
                        if (x < spriteX || x >= spriteX + 8) continue;

                        uint8_t localX = static_cast<uint8_t>(x - spriteX);
                        int row = sprite.getRowInSprite(static_cast<uint8_t>(scanline), spriteHeight);

                        if (row < 0) continue;

                        uint8_t pixelValue = getSpritePixel(sprite, localX, static_cast<uint8_t>(row), i);
                        if (pixelValue == 0) continue;

                        uint8_t color = getSpritePaletteColor(sprite, pixelValue);
                        bool behindBg = sprite.behindBackground();
                        bool isSpriteZero = (i == 0); 

                        return {color, false, behindBg, isSpriteZero};
                    }

                    return {0, true, false, false};
                }


                static constexpr std::array<
                    std::array<uint8_t, 3>,
                        64
                    > nesPalette = {{
                        {{ 84,  84,  84}}, {{  0,  30, 116}}, {{  8,  16, 144}}, {{ 48,   0, 136}},
                        {{ 68,   0, 100}}, {{ 92,   0,  48}}, {{ 84,   4,   0}}, {{ 60,  24,   0}},
                        {{ 32,  42,   0}}, {{  8,  58,   0}}, {{  0,  64,   0}}, {{  0,  60,   0}},
                        {{  0,  50,  60}}, {{  0,   0,   0}}, {{  0,   0,   0}}, {{  0,   0,   0}},

                        {{152, 150, 152}}, {{  8,  76, 196}}, {{ 48,  50, 236}}, {{ 92,  30, 228}},
                        {{136,  20, 176}}, {{160,  20, 100}}, {{152,  34,  32}}, {{120,  60,   0}},
                        {{ 84,  90,   0}}, {{ 40, 114,   0}}, {{  8, 124,   0}}, {{  0, 118,  40}},
                        {{  0, 102, 120}}, {{  0,   0,   0}}, {{  0,   0,   0}}, {{  0,   0,   0}},

                        {{236, 238, 236}}, {{ 76, 154, 236}}, {{120, 124, 236}}, {{176,  98, 236}},
                        {{228,  84, 236}}, {{236,  88, 180}}, {{236, 106, 100}}, {{212, 136,  32}},
                        {{160, 170,   0}}, {{116, 196,   0}}, {{ 76, 208,  32}}, {{ 56, 204, 108}},
                        {{ 56, 180, 204}}, {{ 60,  60,  60}}, {{  0,   0,   0}}, {{  0,   0,   0}},

                        {{236, 238, 236}}, {{168, 204, 236}}, {{188, 188, 236}}, {{212, 178, 236}},
                        {{236, 174, 236}}, {{236, 174, 212}}, {{236, 180, 176}}, {{228, 196, 144}},
                        {{204, 210, 120}}, {{180, 222, 120}}, {{168, 226, 144}}, {{152, 226, 180}},
                        {{160, 214, 228}}, {{160, 162, 160}}, {{  0,   0,   0}}, {{  0,   0,   0}}
                }};
        };
}