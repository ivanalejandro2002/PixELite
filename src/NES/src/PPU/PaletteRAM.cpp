#include <NES/PPU/PaletteRAM.h>

namespace NES
{
    bool PaletteRAM::handles(uint16_t addr) const
    {
        return addr >= 0x3F00 && addr <= 0x3FFF;
    }

    uint8_t PaletteRAM::translateAddress(uint16_t addr) const
    {
        uint8_t index = static_cast<uint8_t> (addr & 0x1F);

        switch(index)
        {
            case 0x10:
                return 0x00;
            case 0x14:
                return 0x04;
            case 0x18:
                return 0x08;
            case 0x1C:
                return 0x0C;
            default:
                return index;
        }
    }

    uint8_t PaletteRAM::read(uint16_t addr)
    {
        return memory[translateAddress(addr)];
    }

    void PaletteRAM::write(uint16_t addr, uint8_t value)
    {
        memory[translateAddress(addr)] = value & 0x3F;
    }
}