#pragma once
#include <NES/Mapper/Mapper.h>
#include <cstdint>

namespace NES {

    class Mapper30 : public Mapper {
        private:
            uint32_t prgBankSelect = 0;      // Banco de 16 KB activo en $8000-$BFFF (bits 0-4)
            uint32_t lastPrgBankOffset = 0;  // Offset fijo al último banco de 16 KB en $C000-$FFFF
            uint32_t chrBankSelect = 0;      // Banco de 8 KB de CHR-RAM activo en $0000-$1FFF (bits 5-6)

            bool oneScreenMode = false;      // true si el cartucho usa Single-Screen dinámico (bit 7)
            Mirroring currentMirroring = Mirroring::SingleScreenLower;

        public:
            Mapper30(uint8_t prgBanks, uint8_t chrBanks, bool isOneScreenDynamic = false)
                : Mapper(prgBanks, chrBanks), oneScreenMode(isOneScreenDynamic)
            {
                lastPrgBankOffset = (prgBanks > 0) ? (prgBanks - 1) * 16384 : 0;
                prgBankSelect = 0;
                chrBankSelect = 0;
                currentMirroring = Mirroring::SingleScreenLower;
            }

            bool cpuMapRead(uint16_t addr, uint32_t& mappedAddr) override {
                if (addr >= 0x8000 && addr <= 0xBFFF) {
                    // Ventana inferior conmutable de 16 KB
                    mappedAddr = (prgBankSelect * 16384) + (addr - 0x8000);
                    return true;
                }
                else if (addr >= 0xC000 && addr <= 0xFFFF) {
                    // Ventana superior fija al último banco de 16 KB
                    mappedAddr = lastPrgBankOffset + (addr - 0xC000);
                    return true;
                }
                return false;
            }

            bool cpuMapWrite(uint16_t addr, uint32_t& mappedAddr, uint8_t data) override {
                if (addr >= 0x8000 && addr <= 0xFFFF) {
                    // Bits 0-4: Selección de banco PRG de 16 KB (hasta 32 bancos = 512 KB)
                    if (prgBanks > 0) {
                        prgBankSelect = (data & 0x1F) % prgBanks;
                    }

                    // Bits 5-6: Selección de banco CHR-RAM de 8 KB (hasta 4 bancos = 32 KB)
                    if (chrBanks > 0) {
                        chrBankSelect = ((data >> 5) & 0x03) % chrBanks;
                    }

                    // Bit 7: Selección de Nametable en modo Single-Screen dinámico
                    if (oneScreenMode) {
                        currentMirroring = (data & 0x80)
                            ? Mirroring::SingleScreenUpper
                            : Mirroring::SingleScreenLower;
                    }

                    return true;
                }
                return false;
            }

            bool ppuMapRead(uint16_t addr, uint32_t& mappedAddr) override {
                if (addr >= 0x0000 && addr <= 0x1FFF) {
                    mappedAddr = (chrBankSelect * 8192) + addr;
                    return true;
                }
                return false;
            }

            bool ppuMapWrite(uint16_t addr, uint32_t& mappedAddr) override {
                if (addr >= 0x0000 && addr <= 0x1FFF) {
                    mappedAddr = (chrBankSelect * 8192) + addr;
                    return true;
                }
                return false;
            }

            bool hasDynamicMirroring() const override {
                return oneScreenMode;
            }

            Mirroring getMapperMirroring() const override {
                return currentMirroring;
            }
    };

}