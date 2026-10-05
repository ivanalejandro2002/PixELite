#pragma once

#include <NES/bus/Bus.h>
#include <NES/PPU/CIRAM.h>
#include <NES/PPU/PaletteRAM.h>
#include <NES/Cartridge/CHRROM.h>

#include <array>
#include <cstdint>
#include <vector>

namespace NES
{
    class PPU2C02
    {
        public:
            PPU2C02(
                std::vector<uint8_t>& chrData,
                CIRAM::Mirroring mirroring
            );

            uint8_t readVRAM(uint16_t addr);
            void writeVRAM(uint16_t addr, uint8_t value);

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

            uint16_t v, t;

            uint8_t fineX;

            bool writeToggle;

            uint8_t readBuffer;
    };
}