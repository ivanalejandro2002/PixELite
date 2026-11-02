#pragma once
#include <NES/Mapper/Mapper.h>

namespace NES {

class Mapper0 : public Mapper {
public:
    Mapper0(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {}

    bool cpuMapRead(uint16_t addr, uint32_t& mappedAddr) override {
        if (addr >= 0x8000 && addr <= 0xFFFF) {
            // Si el juego usa 16KB de PRG, se espeja en la segunda mitad ($C000-$FFFF)
            mappedAddr = addr & (prgBanks > 1 ? 0x7FFF : 0x3FFF);
            return true;
        }
        return false;
    }

    bool cpuMapWrite(uint16_t addr, uint32_t& mappedAddr, uint8_t data) override {
        if (addr >= 0x8000 && addr <= 0xFFFF) {
            return true; // Captura la escritura en el rango de ROM, pero NROM no tiene registros
        }
        return false;
    }

    bool ppuMapRead(uint16_t addr, uint32_t& mappedAddr) override {
        if (addr >= 0x0000 && addr <= 0x1FFF) {
            mappedAddr = addr;
            return true;
        }
        return false;
    }

    bool ppuMapWrite(uint16_t addr, uint32_t& mappedAddr) override {
        if (addr >= 0x0000 && addr <= 0x1FFF) {
            mappedAddr = addr; // Permite escritura si se usa CHR-RAM
            return true;
        }
        return false;
    }
};

} // namespace NES