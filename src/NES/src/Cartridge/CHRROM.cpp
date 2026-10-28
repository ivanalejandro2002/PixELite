#include <NES/Cartridge/CHRROM.h>

namespace NES
{
    CHRROM::CHRROM(const std::vector<uint8_t> &data)
        : memory(data)
    {
        isWritable = false;
    }

    bool CHRROM::handles(uint16_t addr) const
    {
        return addr <= 0x1FFF;
    }

    uint8_t CHRROM::read(uint16_t addr)
    {
        if(addr >= memory.size())
            return 0xFF;

        return memory[addr];
    }

    void CHRROM::write(uint16_t addr, uint8_t value)
    {
        
    }
}