#pragma once

#include <NES/Device.h>
#include <NES/PPU/Mirroring.h>
#include <array>
#include <cstdint>

namespace NES
{
    class CIRAM : public Device
    {
        public:

            explicit CIRAM(Mirroring mirroring);

            bool handlesRead(uint16_t addr) const override;
            bool handlesWrite(uint16_t addr) const override;

            
            uint8_t read(uint16_t addr) override;
            void write(uint16_t addr, uint8_t value) override;

            void setMirroring(Mirroring mirroring);

        private:
            std::array<uint8_t, 2048> memory{};

            Mirroring mirroring;

            uint16_t translateAddress(uint16_t addr) const;
    };
}