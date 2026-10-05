#include <NES/PPU/CIRAM.h>

namespace NES
{
    CIRAM::CIRAM(Mirroring mirroring)
        : mirroring(mirroring)
    {    
    }

    bool CIRAM::handles(uint16_t addr) const
    {
        return addr >= 0x2000 && addr <= 0x3EFF;
    }

    void CIRAM::setMirroring(Mirroring mirroring)
    {
        this->mirroring = mirroring;
    }

    uint16_t CIRAM::translateAddress(uint16_t address) const
    {
        if(address >= 0x3000)add -= 0x1000;

        const uint16_t offset = (addr & 0x1FFF);

        const uint16_t nametable = offset >> 10;
        const uint16_t position = offset & 0x03FF;

        uint16_t physicalNametable = 0;

        switch(mirroring)
        {
            case Mirroring::Vertical:
                physicalNametable = nametable & 0x01;
                break;

            case Mirroring::Horizontal:
                physicalNametable = nametable >> 1;
                break;

            case Mirroring::SingleScreenLower:
                physicalNametable = 0;
                break;
            
            case Mirroring::SingleScreenUpper:
                physicalNametable = 1;
                break;
        }

        return physicalNametable * 0x0400 + position;
    }

    uint8_t CIRAM::read(uint16_t addr)
    {
        return memory[translateAddress(addr)];
    }

    void CIRAM::write(uint16_t addr, uint8_t value)
    {
        memory[translateAddress(addr)] = value;
    }
}