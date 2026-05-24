#include <NES/Memory/RAM.h>

namespace NES
{
    bool RAM::handles(uint16_t addr)const
    {
        return addr <= 0x1FFF;
    }

    uint8_t RAM::read(uint16_t addr)
    {
        return memory[addr];
    }

    void RAM::write(uint16_t addr, uint8_t value)
    {
        memory[addr] = value;
    }
}