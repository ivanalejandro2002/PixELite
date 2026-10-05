#include <NES/PPU/PPU2C02.h>

namespace NES
{
    PPU2C02::PPU2C02(
        const std::vector<uint8_t> &chrData,
        CIRAM::Mirroring mirroring
    )
        : chr(chrData),
            ciram(CIRAM::Mirroring::Horizontal)
    {
        bus.addDevice(&chr);
        bus.addDevice(&ciram);
        bus.addDevice(&palette);

        t = v = 0;
        fineX = 0;
        writeToggle = false;

        readBuffer = 0;
    }

    uint8_t PPU2C02::readVRAM(uint16_t addr)
    {
        return bus.read(addr);
    }

    void PPU2C02::writeVRAM(uint16_t addr, uint8_t value)
    {
        bus.write(addr, value);
    }

    void PPU2C02::writePPUCTRL(uint8_t value)
    {
        
    }

    void PPU2C02::writePPUMASK(uint8_t value)
    {
        PPUMASK = value;
    }

    void PPU2C02::writeOAMADDR(uint8_t value)
    {
        OAMADDR = value;
    }

    uint8_t PPU2C02::readOAMDATA()
    {
        return OAM[OAMADDR];
    }

    uint8_t PPU2C02::writeOAMDATA(uint8_t value)
    {
        OAM[OAMADDR] = value;
        OAMADDR++;
    }

    void PPU2C02::writePPUSCROLL(uint8_t value)
    {
        if(!writeToggle)
        {
            fineX = value & 0x07;

            t &= 0xFFE0;
            t |= (static_cast<uint16_t>(value) >> 3) & 0x001F;

            writeToggle = true;
        }else
        {
            t &= 0x8C1F;
            t |= (static_cast<uint16_t>(value) & 0x07) << 12;
            t |= (static_cast<uint16_t>(value) >> 3) << 5;

            writeToggle = false;
        }
    }

    void PPU2C02::writePPUADDR(uint8_t value)
    {
        if(!writeToggle)
        {
            t &= 0x00FF;

            t |= (static_cast<uint16_t>(value) & 0x3F) << 8;
            writeToggle = true;
        }else
        {
            t &= 0x7F00;
            t |= value;

            v = t;

            writeToggle = false;
        }
    }

    uint8_t PPU2C02::readPPUDATA()
    {
        uint8_t value = readVRAM(v);

        if(PPUCTRL & 0x04)
            v += 32;
        else v += 1;

        return value;
    }

    void PPU2C02::writePPUDATA(uint8_t value)
    {
        writeVRAM(v, value);

        if(PPUCTRL & 0x04)
            v += 32;
        else v += 1;
    }

    void PPU2C02::getVRAMAddress() const
    {
        return v;
    }

    void PPU::reset()
    {
        w = false;
    }
}