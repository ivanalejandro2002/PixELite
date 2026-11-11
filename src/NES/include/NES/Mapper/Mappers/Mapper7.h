#pragma once
#include <NES/Mapper/Mapper.h>
#include <cstdint>

namespace NES {

class Mapper7 : public Mapper {
private:
    uint32_t prgBankOffset = 0; // Offset físico en bytes del banco actual de 32 KB
    Mirroring currentMirroring = Mirroring::SingleScreenLower;

public:
    Mapper7(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {
        // prgBanks viene del header iNES en unidades de 16 KB.
        // Al encender, arrancamos en el primer banco de 32 KB.
        prgBankOffset = 0;
        currentMirroring = Mirroring::SingleScreenLower;
    }

    bool cpuMapRead(uint16_t addr, uint32_t& mappedAddr) override {
        if (addr >= 0x8000 && addr <= 0xFFFF) {
            // Ventana única de 32 KB ($8000 - $FFFF)
            mappedAddr = prgBankOffset + (addr - 0x8000);
            return true;
        }
        return false;
    }

    bool cpuMapWrite(uint16_t addr, uint32_t& mappedAddr, uint8_t data) override {
        if (addr >= 0x8000 && addr <= 0xFFFF) {
            // Bits 0-2 (o 0-3 en placas compatibles expandidas): Selección de banco de 32 KB.
            // Como prgBanks está en bloques de 16 KB, el total de bloques de 32 KB es (prgBanks / 2).
            uint32_t total32kBanks = (prgBanks >= 2) ? (prgBanks / 2) : 1;
            uint32_t selectedBank = (data & 0x07) % total32kBanks;

            prgBankOffset = selectedBank * 32768; // 32 KB = 0x8000 bytes

            // Bit 4: Selección de página de VRAM (Single-Screen Mirroring)
            if (data & 0x10) {
                currentMirroring = Mirroring::SingleScreenUpper;
            } else {
                currentMirroring = Mirroring::SingleScreenLower;
            }

            return true;
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
            // AxROM utiliza 8 KB de CHR-RAM directos
            mappedAddr = addr;
            return true;
        }
        return false;
    }

    bool hasDynamicMirroring() const override { return true; }

    Mirroring getMapperMirroring() const override {
        return currentMirroring;
    }
};

}