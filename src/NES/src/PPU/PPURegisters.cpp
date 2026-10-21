#include <NES/PPU/PPURegisters.h>
#include <NES/PPU/PPU2C02.h>

namespace NES
{
    PPURegisters::PPURegisters(PPU2C02& ppu)
        : ppu(ppu)
    {
        isWritable = true;
    }

    bool PPURegisters::handles(uint16_t addr) const
    {
        return addr >= 0x2000 &&
               addr <= 0x3FFF;
    }

    uint8_t PPURegisters::read(uint16_t addr)
    {
        return ppu.readRegister(addr);
    }

    void PPURegisters::write(
        uint16_t addr,
        uint8_t value
    )
    {
        ppu.writeRegister(addr, value);
    }
}