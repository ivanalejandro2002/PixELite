#pragma once
#include <NES/Mapper/Mapper.h>
#include <algorithm>

#include <iostream>

namespace NES {

class Mapper1 : public Mapper {
private:
    // Registro de desplazamiento de 5 bits para MMC1
    uint8_t shiftRegister = 0x10; // Arranca con el bit 4 en 1 (indica el inicio/fin del shift)
    uint8_t shiftCount = 0;

    // Registros internos del MMC1
    uint8_t control = 0x0C;       // Registro de control ($8000-$9FFF) -> Valor por defecto inicializa PRG mode 3
    uint8_t chrBank0 = 0;         // Banco CHR 0 ($A000-$BFFF)
    uint8_t chrBank1 = 0;         // Banco CHR 1 ($C000-$DFFF)
    uint8_t prgBank = 0;          // Banco PRG ($E000-$FFFF)

    // Offsets calculados para optimizar los mapeos en tiempo de lectura
    uint32_t prgOffsetLow = 0;
    uint32_t prgOffsetHigh = 0;
    uint32_t chrOffsetLow = 0;
    uint32_t chrOffsetHigh = 0;

    // Función interna para actualizar los offsets físicos de memoria según los registros actuales
    void updateOffsets() {
        uint8_t prgMode = (control >> 2) & 0x03;
        uint32_t last16kBank = (prgBanks > 0) ? (prgBanks - 1) : 0;

        switch (prgMode) {
            case 0:
            case 1: // Modo 32KB (cambio de 32 KB completos en $8000-$FFFF)
                {
                    uint32_t baseBank = (prgBank & 0x0E); // Ignora el bit menos significativo
                    uint32_t safe32kBank = (prgBanks > 0) ? (baseBank % prgBanks) : 0;
                    prgOffsetLow = safe32kBank * 16384;
                    // El segundo banco de 16KB de ese bloque de 32KB
                    prgOffsetHigh = ((safe32kBank + 1) % prgBanks) * 16384; 
                }
                break;
            case 2: // Modo 16KB: Banco fijo de 16KB en $8000, conmutable en $C000
                prgOffsetLow = 0; // Primer banco fijo
                prgOffsetHigh = (prgBanks > 0) ? ((prgBank % prgBanks) * 16384) : 0;
                break;
            case 3: // Modo 16KB: Conmutable en $8000, último banco fijo en $C000
                prgOffsetLow = (prgBanks > 0) ? ((prgBank % prgBanks) * 16384) : 0;
                prgOffsetHigh = last16kBank * 16384;
                break;
        }

        // --- 2. CONFIGURACIÓN DE CHR (Ventanas de 4 KB o 8 KB) ---
        uint8_t chrMode = (control & 0x10) >> 4;
        
        // Protección para los bancos de gráficos CHR
        uint32_t safeChrBank0 = (chrBanks > 0) ? (chrBank0 % chrBanks) : 0;
        uint32_t safeChrBank1 = (chrBanks > 0) ? (chrBank1 % chrBanks) : 0;

        if (chrMode == 0) {
            uint32_t bank8k = (safeChrBank0 & 0x1E) * 4096;
            // Asegurarnos de que el banco de 8KB no rebase el límite
            if (chrBanks > 0 && bank8k >= chrBanks * 4096) bank8k = 0;
            chrOffsetLow = bank8k;
            chrOffsetHigh = bank8k + 4096;
        } else {
            chrOffsetLow = safeChrBank0 * 4096;
            chrOffsetHigh = safeChrBank1 * 4096;
        }
    }

public:
    Mapper1(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {
        // Inicialización estándar del MMC1 al encender
        control = 0x0C; // Modo PRG 3, Modo CHR 8KB, Mirroring por defecto
        updateOffsets();
    }

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
            // Si el bit 7 está en 1, el MMC1 se reinicia por completo
            if (data & 0x80) {
                shiftRegister = 0x10;
                shiftCount = 0;
                control = (control & ~0x1C) | 0x0C; // Asegura PRG mode 3 por defecto
                updateOffsets();
                return true;
            }

            // Desplazamiento normal de 5 bits
            bool bit0 = (data & 0x01) != 0;
            shiftRegister = (shiftRegister >> 1) | (static_cast<uint8_t>(bit0) << 4);
            shiftCount++;

            if (shiftCount == 5) {
                uint8_t finalValue = shiftRegister;
                
                // Extraer los bits 14 y 13 para saber exactamente a qué ventana de 8KB pertenece la dirección
                uint8_t targetRegister = (addr >> 13) & 0x03;

                switch (targetRegister) {
                    case 0: // $8000 - $9FFF
                        control = finalValue; 
                        break;
                    case 1: // $A000 - $BFFF
                        chrBank0 = finalValue; 
                        break;
                    case 2: // $C000 - $DFFF
                        chrBank1 = finalValue; 
                        break;
                    case 3: // $E000 - $FFFF
                        prgBank = finalValue; 
                        break;
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
            // Primera ventana PPU de 4 KB
            mappedAddr = chrOffsetLow + addr;
            return true;
        }
        else if (addr >= 0x1000 && addr <= 0x1FFF) {
            // Segunda ventana PPU de 4 KB
            mappedAddr = chrOffsetHigh + (addr - 0x1000);
            return true;
        }
        return false;
    }

    bool ppuMapWrite(uint16_t addr, uint32_t& mappedAddr) override {
        // Soporte para CHR-RAM en caso de que el juego lo requiera
        if (addr >= 0x0000 && addr <= 0x1FFF) {
            mappedAddr = addr;
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

} // namespace NES