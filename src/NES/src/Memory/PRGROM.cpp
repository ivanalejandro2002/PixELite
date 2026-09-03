#include <NES/Memory/PRGROM.h>

#include <stdexcept>

namespace NES
{
    PRGROM::PRGROM(const std::vector<uint8_t>& data)
        : memory(data)
    {
        if (memory.empty())
        {
            throw std::invalid_argument(
                "PRG-ROM no puede estar vacía"
            );
        }

        isWritable = false;
    }


    bool PRGROM::handles(uint16_t addr) const
    {
        return addr >= 0x8000;
    }


    uint8_t PRGROM::read(uint16_t addr)
    {
        uint16_t offset = addr - 0x8000;

        if (memory.size() == 0x4000)
        {
            offset &= 0x0FFF;
        }

        return memory[offset];
    }


    void PRGROM::write(
        uint16_t addr,
        uint8_t value
    )
    {
        // PRG-ROM no permite escritura.
    }
}