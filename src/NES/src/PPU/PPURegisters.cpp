#include <NES/PPU/PPURegisters.h>
#include <NES/PPU/PPU2C02.h>

namespace NES
{
    PPURegisters::PPURegisters(PPU2C02& ppu)
        : ppu(ppu)
    {
    }

    bool PPURegisters::handles(uint16_t addr) const
    {
        return addr >= 0x2000 &&
               addr <= 0x2007;
    }

    uint8_t PPURegisters::read(uint16_t addr)
    {
        switch(addr)
        {
            case 0x2002:
                return ppu.readPPUSTATUS();
            
            case 0x2004:
                return ppu.readOAMDATA();

            case 0x2007:
                return ppu.readPPUDATA();

            default:
                return 0;
        }
    }

    void PPURegisters::write(
        uint16_t addr,
        uint8_t value
    )
    {
        switch(addr)
        {
            case 0x2000:
                ppu.writePPUCTRL(value);
                break;
            
            case 0x2001:
                ppu.writePPUMASK(value);
                break;
            
            case 0x2003:
                ppu.writeOAMADDR(value);
                break;
            
            case 0x2004:
                ppu.writeOAMDATA(value);

            case 0x2005:
                ppu.writePPUSCROLL(value);
                break;

            case 0x2006:
                ppu.writePPUADDR(value);
                break;

            case 0x2007:
                ppu.writePPUDATA(value);
                break;
        }
    }
}