#pragma once

#include <NES/Device.h>
#include <cstdint>

namespace NES
{
    class PPU2C02;

    class PPURegisters : public Device
    {
    public:
        explicit PPURegisters(PPU2C02& ppu);

        bool handles(uint16_t addr) const override;

        uint8_t read(uint16_t addr) override;
        void write(uint16_t addr, uint8_t value) override;

    private:
        PPU2C02& ppu;
    };
}