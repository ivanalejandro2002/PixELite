#pragma once

#include <NES/Device.h>
#include <array>
#include <cstdint>

namespace NES
{
    class PaletteRAM : public Device
    {
        public:
            PaletteRAM();

            bool handles(uint16_t addr) const override;

            uint8_t read(uint16_t addr) override;
            void write(uint16_t addr, uint8_t value) override;
        private:
            std::array<uint8_t, 32> memory{};

            uint8_t translateAddress(uint16_t addr) const;
    };
}