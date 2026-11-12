#pragma once
#include <NES/Mapper/Mapper.h>

namespace NES {

class Mapper2 : public Mapper {
private:
    uint32_t prgBankSelect = 0;      // Banco activo para la ventana inferior ($8000-$BFFF)
    uint32_t lastPrgBankOffset = 0;  // Offset fijo para la ventana superior ($C000-$FFFF)

public:
    Mapper2(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {
        // prgBanks representa cuántos bloques de 16KB tiene la ROM.
        // El último banco siempre se queda fijo arriba.
        lastPrgBankOffset = (prgBanks > 0) ? (prgBanks - 1) * 16384 : 0;
        prgBankSelect = 0; // Arranca apuntando al primer banco al encender
    }

    void reset() override {
        prgBankSelect = 0;
    }

    bool cpuMapRead(uint16_t addr, uint32_t& mappedAddr) override {
        if (addr >= 0x8000 && addr <= 0xBFFF) {
            // Ventana inferior conmutable (16 KB)
            mappedAddr = (prgBankSelect * 16384) + (addr - 0x8000);
            return true;
        }
        else if (addr >= 0xC000 && addr <= 0xFFFF) {
            // Ventana superior fija al último banco (16 KB)
            mappedAddr = lastPrgBankOffset + (addr - 0xC000);
            return true;
        }
        return false;
    }

    bool cpuMapWrite(uint16_t addr, uint32_t& mappedAddr, uint8_t data) override {
        if (addr >= 0x8000 && addr <= 0xFFFF) {
            // Cualquier escritura en el rango $8000-$FFFF actualiza el banco inferior.
            // Usamos una máscara binaria basada en prgBanks para evitar desbordamientos.
            prgBankSelect = data & (prgBanks - 1);
            return true;
        }
        return false;
    }

    bool ppuMapRead(uint16_t addr, uint32_t& mappedAddr) override {
        if (addr >= 0x0000 && addr <= 0x1FFF) {
            // UxROM usa típicamente CHR-RAM, mapeo directo de 8KB
            mappedAddr = addr;
            return true;
        }
        return false;
    }

    bool ppuMapWrite(uint16_t addr, uint32_t& mappedAddr) override {
        if (addr >= 0x0000 && addr <= 0x1FFF) {
            // Permite escritura para gestionar la CHR-RAM del cartucho
            mappedAddr = addr;
            return true;
        }
        return false;
    }
};

} // namespace NES