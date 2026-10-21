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
        };
}