#pragma once
#include <NES/Mapper/Mapper.h>
#include <array>
#include <cstdint>

namespace NES {

class Mapper4 : public Mapper {
private:
    // Selección de registro objetivo ($8000-$9FFE, par)
    uint8_t targetRegister = 0x00;
    bool prgBankMode = false; // Bit 6: false = $8000 conmutable / $C000 fijo; true = $C000 conmutable / $8000 fijo
    bool chrInversion = false; // Bit 7: false = 2x2KB en $0000 y 4x1KB en $1000; true = invertido

    // 8 registros internos de bancos (R0-R5 para CHR, R6-R7 para PRG)
    std::array<uint32_t, 8> registers{};

    // Offsets precalculados (4 ventanas PRG de 8KB y 8 ventanas CHR de 1KB)
    std::array<uint32_t, 4> prgOffsets{};
    std::array<uint32_t, 8> chrOffsets{};

    // Mirroring controlado por $A000 (par)
    Mirroring currentMirroring = Mirroring::Vertical;

    // Control de PRG-RAM ($A001, impar)
    bool prgRamEnabled = true;
    bool prgRamWriteProtect = false;

    // Estado del contador de scanlines IRQ
    uint8_t irqLatch = 0x00;
    uint8_t irqCounter = 0x00;
    bool irqEnabled = false;
    bool irqReloadPending = false;
    bool irqActive = false;

    // Detector de flanco de subida en PPU A12
    int a12LowCount = 0;

    void updateOffsets() {
        // --- 1. CONFIGURACIÓN DE PRG-ROM (Bancos de 8 KB) ---
        // prgBanks viene del header iNES en bloques de 16KB -> Total de bancos de 8KB = prgBanks * 2
        uint32_t total8kPrgBanks = (prgBanks > 0) ? (prgBanks * 2) : 2;
        uint32_t secondLastBank = (total8kPrgBanks >= 2) ? (total8kPrgBanks - 2) : 0;
        uint32_t lastBank       = (total8kPrgBanks >= 1) ? (total8kPrgBanks - 1) : 0;

        uint32_t r6 = (registers[6] & 0x3F) % total8kPrgBanks;
        uint32_t r7 = (registers[7] & 0x3F) % total8kPrgBanks;

        if (prgBankMode) {
            // $8000-$9FFF fijo al penúltimo banco; $C000-$DFFF conmutable con R6
            prgOffsets[0] = secondLastBank * 0x2000;
            prgOffsets[1] = r7 * 0x2000;
            prgOffsets[2] = r6 * 0x2000;
            prgOffsets[3] = lastBank * 0x2000;
        } else {
            // $8000-$9FFF conmutable con R6; $C000-$DFFF fijo al penúltimo banco
            prgOffsets[0] = r6 * 0x2000;
            prgOffsets[1] = r7 * 0x2000;
            prgOffsets[2] = secondLastBank * 0x2000;
            prgOffsets[3] = lastBank * 0x2000;
        }

        // --- 2. CONFIGURACIÓN DE CHR (8 ventanas de 1 KB) ---
        // chrBanks viene en bloques de 8KB -> Total de bancos de 1KB = chrBanks * 8
        uint32_t total1kChrBanks = (chrBanks > 0) ? (chrBanks * 8) : 8;

        // R0 y R1 seleccionan bancos de 2KB (se ignora el bit 0)
        uint32_t r0_low  = (registers[0] & 0xFE) % total1kChrBanks;
        uint32_t r0_high = ((registers[0] & 0xFE) + 1) % total1kChrBanks;
        uint32_t r1_low  = (registers[1] & 0xFE) % total1kChrBanks;
        uint32_t r1_high = ((registers[1] & 0xFE) + 1) % total1kChrBanks;

        // R2 a R5 seleccionan bancos de 1KB
        uint32_t r2 = registers[2] % total1kChrBanks;
        uint32_t r3 = registers[3] % total1kChrBanks;
        uint32_t r4 = registers[4] % total1kChrBanks;
        uint32_t r5 = registers[5] % total1kChrBanks;

        if (chrInversion) {
            // $0000-$0FFF: 4 bancos de 1KB (R2, R3, R4, R5)
            // $1000-$1FFF: 2 bancos de 2KB (R0, R1)
            chrOffsets[0] = r2 * 0x0400;
            chrOffsets[1] = r3 * 0x0400;
            chrOffsets[2] = r4 * 0x0400;
            chrOffsets[3] = r5 * 0x0400;
            chrOffsets[4] = r0_low * 0x0400;
            chrOffsets[5] = r0_high * 0x0400;
            chrOffsets[6] = r1_low * 0x0400;
            chrOffsets[7] = r1_high * 0x0400;
        } else {
            // $0000-$0FFF: 2 bancos de 2KB (R0, R1)
            // $1000-$1FFF: 4 bancos de 1KB (R2, R3, R4, R5)
            chrOffsets[0] = r0_low * 0x0400;
            chrOffsets[1] = r0_high * 0x0400;
            chrOffsets[2] = r1_low * 0x0400;
            chrOffsets[3] = r1_high * 0x0400;
            chrOffsets[4] = r2 * 0x0400;
            chrOffsets[5] = r3 * 0x0400;
            chrOffsets[6] = r4 * 0x0400;
            chrOffsets[7] = r5 * 0x0400;
        }
    }

    void clockIrqCounter() {
        if (irqCounter == 0 || irqReloadPending) {
            irqCounter = irqLatch;
            irqReloadPending = false;
        } else {
            irqCounter--;
        }

        // Revisión estándar MMC3B/MMC3C (dispara cuando llega a 0)
        if (irqCounter == 0 && irqEnabled) {
            irqActive = true;
        }
    }

public:
    Mapper4(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {
        registers.fill(0);
        registers[0] = 0;
        registers[1] = 2;
        registers[2] = 4;
        registers[3] = 5;
        registers[4] = 6;
        registers[5] = 7;
        registers[6] = 0;
        registers[7] = 1;
        updateOffsets();
    }

    bool cpuMapRead(uint16_t addr, uint32_t& mappedAddr) override {
        if (addr >= 0x8000 && addr <= 0xFFFF) {
            uint8_t window = (addr - 0x8000) / 0x2000; // 4 ventanas de 8KB (0..3)
            mappedAddr = prgOffsets[window] + (addr & 0x1FFF);
            return true;
        }
        return false;
    }

    bool cpuMapWrite(uint16_t addr, uint32_t& mappedAddr, uint8_t data) override {
        if (addr < 0x8000) return false;

        bool isEven = (addr & 0x0001) == 0;

        if (addr >= 0x8000 && addr <= 0x9FFF) {
            if (isEven) {
                // Bank Select ($8000-$9FFE, par)
                targetRegister = data & 0x07;
                prgBankMode    = (data & 0x40) != 0;
                chrInversion   = (data & 0x80) != 0;
                updateOffsets();
            } else {
                // Bank Data ($8001-$9FFF, impar)
                registers[targetRegister] = data;
                updateOffsets();
            }
            return true;
        }
        else if (addr >= 0xA000 && addr <= 0xBFFF) {
            if (isEven) {
                // Mirroring ($A000-$BFFE, par): 0 = Vertical, 1 = Horizontal
                currentMirroring = (data & 0x01) ? Mirroring::Horizontal : Mirroring::Vertical;
            } else {
                // PRG-RAM Protect ($A001-$BFFF, impar)
                prgRamWriteProtect = (data & 0x40) != 0;
                prgRamEnabled      = (data & 0x80) != 0;
            }
            return true;
        }
        else if (addr >= 0xC000 && addr <= 0xDFFF) {
            if (isEven) {
                // IRQ Latch ($C000-$DFFE, par)
                irqLatch = data;
            } else {
                // IRQ Reload ($C001-$DFFF, impar)
                irqCounter = 0;
                irqReloadPending = true;
            }
            return true;
        }
        else if (addr >= 0xE000 && addr <= 0xFFFF) {
            if (isEven) {
                // IRQ Disable ($E000-$FFFE, par): Deshabilita y reconoce (acknowledge) IRQ pendientes
                irqEnabled = false;
                irqActive = false;
            } else {
                // IRQ Enable ($E001-$FFFF, impar)
                irqEnabled = true;
            }
            return true;
        }

        return false;
    }

    bool ppuMapRead(uint16_t addr, uint32_t& mappedAddr) override {
        if (addr >= 0x0000 && addr <= 0x1FFF) {
            uint8_t window = addr / 0x0400; // 8 ventanas de 1KB (0..7)
            mappedAddr = chrOffsets[window] + (addr & 0x03FF);
            return true;
        }
        return false;
    }

    bool ppuMapWrite(uint16_t addr, uint32_t& mappedAddr) override {
        if (addr >= 0x0000 && addr <= 0x1FFF) {
            uint8_t window = addr / 0x0400;
            mappedAddr = chrOffsets[window] + (addr & 0x03FF);
            return true;
        }
        return false;
    }

    void notifyPpuaddress(uint16_t addr) override {
        if (addr >= 0x3F00) return;

        bool currA12 = (addr & 0x1000) != 0;

        if (!currA12) {
            if (a12LowCount < 30) {
                a12LowCount++;
            }
        } else {
            // En backgroundFetch ($1000), A12 solo baja 2 accesos (NT + Attr).
            // En la ventana de sprites (dots 257-320) baja 16 accesos seguidos.
            if (a12LowCount >= 6) {
                clockIrqCounter();
            }
            a12LowCount = 0;
        }
    }

    bool hasDynamicMirroring() const override { return true; }

    Mirroring getMapperMirroring() const override {
        return currentMirroring;
    }

    bool irqState() const override {
        return irqActive;
    }

    void irqClear() override {
        irqActive = false;
    }
};

}