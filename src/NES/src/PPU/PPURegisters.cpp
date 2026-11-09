#include <NES/PPU/PPURegisters.h>
#include <NES/PPU/PPU2C02.h>

namespace NES
{
    PPURegisters::PPURegisters(PPU2C02& ppu)
        : ppu(ppu)
    {
        isWritable = true;
    }

    bool PPURegisters::handlesRead(uint16_t addr) const
    {
        return (addr >= 0x2000 && addr <= 0x3FFF) || addr == 0x4014;
    }

    bool PPURegisters::handlesWrite(uint16_t addr) const
    {
        return (addr >= 0x2000 && addr <= 0x3FFF) || addr == 0x4014;
    }

    uint8_t PPURegisters::read(uint16_t addr)
    {
        uint16_t normalizedAddr = 0x2000 + (addr & 0x0007);
        return ppu.readRegister(normalizedAddr);
    }

    void PPURegisters::write(
        uint16_t addr,
        uint8_t value
    )
    {
        if(addr == 0x4014)
        {
            ppu.getMainBus()->triggerOAMDMA(value);
            return;
        }
        uint16_t normalizedAddr = 0x2000 + (addr & 0x0007);
        ppu.writeRegister(normalizedAddr, value);
        
    }

    void PPURegisters::connectMainBus(Bus *bus) {ppu.connectMainBus(bus);}
}