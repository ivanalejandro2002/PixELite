#pragma once
#include <NES/Mapper/Mapper.h>
#include <algorithm>
#include <iostream>

namespace NES {

class Mapper1 : public Mapper {
private:
    uint8_t shiftRegister = 0x10;
    uint8_t shiftCount = 0;

    uint8_t control = 0x0C;
    uint8_t chrBank0 = 0;
    uint8_t chrBank1 = 0;
    uint8_t prgBank = 0;

    uint32_t prgOffsetLow = 0;
    uint32_t prgOffsetHigh = 0;
    uint32_t chrOffsetLow = 0;
    uint32_t chrOffsetHigh = 0;

    void updateOffsets() {
        // --- 1. CONFIGURACIÓN DE PRG (Bancos de 16 KB) ---
        uint8_t prgMode = (control >> 2) & 0x03;
        uint32_t last16kBank = (prgBanks > 0) ? (prgBanks - 1) : 0;
        uint32_t selectedPrg = prgBank & 0x0F; // Ignorar bit 4 (WRAM disable)

        switch (prgMode) {
            case 0:
            case 1: // Modo 32KB (cambio de 32 KB completos en $8000-$FFFF)
                {
                    uint32_t baseBank = (selectedPrg & 0x0E);
                    uint32_t safe32kBank = (prgBanks > 0) ? (baseBank % prgBanks) : 0;
                    prgOffsetLow = safe32kBank * 16384;
                    prgOffsetHigh = ((safe32kBank + 1) % prgBanks) * 16384;
                }
                break;
            case 2: // Modo 16KB: Banco fijo de 16KB en $8000, conmutable en $C000
                prgOffsetLow = 0;
                prgOffsetHigh = (prgBanks > 0) ? ((selectedPrg % prgBanks) * 16384) : 0;
                break;
            case 3: // Modo 16KB: Conmutable en $8000, último banco fijo en $C000
                prgOffsetLow = (prgBanks > 0) ? ((selectedPrg % prgBanks) * 16384) : 0;
                prgOffsetHigh = last16kBank * 16384;
                break;
        }

        // --- 2. CONFIGURACIÓN DE CHR (Ventanas de 4 KB o 8 KB) ---
        uint8_t chrMode = (control & 0x10) >> 4;

        // Total de bancos de 4KB disponibles (cada chrBank de iNES mide 8KB = 2 bancos de 4KB)
        uint32_t total4kBanks = (chrBanks > 0) ? (chrBanks * 2) : 2;

        if (chrMode == 0) {
            // Modo 8KB: usa chrBank0 ignorando el bit 0
            uint32_t bank4k = (chrBank0 & 0x1E) % total4kBanks;
            chrOffsetLow = bank4k * 4096;
            chrOffsetHigh = chrOffsetLow + 4096;
        } else {
            // Modo 4KB: dos ventanas independientes de 4KB
            uint32_t safeChrBank0 = (chrBank0 & 0x1F) % total4kBanks;
            uint32_t safeChrBank1 = (chrBank1 & 0x1F) % total4kBanks;
            chrOffsetLow = safeChrBank0 * 4096;
            chrOffsetHigh = safeChrBank1 * 4096;
        }
    }

public:
    Mapper1(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {
        control = 0x0C;
        chrBank0 = 0;
        chrBank1 = 1; // Inicializar banco alto en el segundo bloque de 4KB por defecto
        updateOffsets();
    }

    bool hasDynamicMirroring() const override { return true; }

    bool cpuMapRead(uint16_t addr, uint32_t& mappedAddr) override {
        if (addr >= 0x8000 && addr <= 0xBFFF) {
            mappedAddr = prgOffsetLow + (addr - 0x8000);
            return true;
        }
        if (addr >= 0xC000 && addr <= 0xFFFF) {
            mappedAddr = prgOffsetHigh + (addr - 0xC000);
            return true;
        }
        return false;
    }

    bool cpuMapWrite(uint16_t addr, uint32_t& mappedAddr, uint8_t data) override {
        if (addr >= 0x8000 && addr <= 0xFFFF) {
            if (data & 0x80) {
                shiftRegister = 0x10;
                shiftCount = 0;
                control |= 0x0C; // Solo activa bits 2 y 3 (PRG mode 3), sin borrar CHR mode
                updateOffsets();
                return true;
            }

            bool bit0 = (data & 0x01) != 0;
            shiftRegister = (shiftRegister >> 1) | (static_cast<uint8_t>(bit0) << 4);
            shiftCount++;

            if (shiftCount == 5) {
                uint8_t finalValue = shiftRegister & 0x1F;
                uint8_t targetRegister = (addr >> 13) & 0x03;

                switch (targetRegister) {
                    case 0: control  = finalValue; break; // $8000 - $9FFF
                    case 1: chrBank0 = finalValue; break; // $A000 - $BFFF
                    case 2: chrBank1 = finalValue; break; // $C000 - $DFFF
                    case 3: prgBank  = finalValue; break; // $E000 - $FFFF
                }

                shiftRegister = 0x10;
                shiftCount = 0;
                updateOffsets();
            }
            return true;
        }
        return false;
    }

    bool ppuMapRead(uint16_t addr, uint32_t& mappedAddr) override {
        if (addr >= 0x0000 && addr <= 0x0FFF) {
            mappedAddr = chrOffsetLow + addr;
            return true;
        }
        else if (addr >= 0x1000 && addr <= 0x1FFF) {
            mappedAddr = chrOffsetHigh + (addr - 0x1000);
            return true;
        }
        return false;
    }

    bool ppuMapWrite(uint16_t addr, uint32_t& mappedAddr) override {
        // Sin "if (chrBanks == 0)" porque Cartridge ya asigna chrBanks = 1 para CHR-RAM
        if (addr >= 0x0000 && addr <= 0x0FFF) {
            mappedAddr = chrOffsetLow + addr;
            return true;
        }
        else if (addr >= 0x1000 && addr <= 0x1FFF) {
            mappedAddr = chrOffsetHigh + (addr - 0x1000);
            return true;
        }
        return false;
    }

    Mirroring getMapperMirroring() const override {
        uint8_t mirroringMode = control & 0x03;
        switch (mirroringMode) {
            case 0: return Mirroring::SingleScreenLower;
            case 1: return Mirroring::SingleScreenUpper;
            case 2: return Mirroring::Vertical;
            case 3: return Mirroring::Horizontal;
            default: return Mirroring::Vertical;
        }
    }
};

}