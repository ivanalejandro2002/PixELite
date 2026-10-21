#pragma once

#include <NES/Bus/Bus.h>
#include <NES/PPU/CIRAM.h>
#include <NES/PPU/PaletteRAM.h>
#include <NES/Cartridge/CHRROM.h>
#include <Graphics/FrameBuffer.h>

#include <array>
#include <cstdint>
#include <vector>

namespace NES
{
        class PPU2C02
        {
            public:
                PPU2C02(
                    const std::vector<uint8_t> &chrData,
                    CIRAM::Mirroring mirroring
                );

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

                void reset();

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

            private:
                Bus bus;

                CHRROM chr;
                CIRAM ciram;
                PaletteRAM palette;

                uint8_t PPUCTRL = 0;
                uint8_t PPUMASK = 0;
                uint8_t PPUSTATUS = 0;

                uint8_t OAMADDR = 0;

                std::array<int, 256> OAM{};

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
                uint16_t cycle = 0;

                void incrementCoarseX();
                void incrementY();
                void transferAddressX();
                void transferAddressY();

                void loadBackgroundShifters();
                void updateBackgroundShifters();
                void renderBackgroundPixel();

                void backgroundFetch();


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