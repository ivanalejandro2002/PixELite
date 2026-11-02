#pragma once
#include <NES/Device.h>
#include <NES/Cartridge/Cartridge.h>

namespace NES {
    class PPUCartridgeBridge : public Device {
    private:
        Cartridge* cartridge;

    public:
        PPUCartridgeBridge(Cartridge* cart) : cartridge(cart) {
            isWritable = true; // Permite CHR-RAM si el cartucho lo usa
        }

        bool handles(uint16_t addr) const override {
            // El espacio de patrones de la PPU va de 0x0000 a 0x1FFF
            return addr >= 0x0000 && addr <= 0x1FFF;
        }

        uint8_t read(uint16_t addr) override {
            if (cartridge) {
                return cartridge->ppuRead(addr);
            }
            return 0;
        }

        void write(uint16_t addr, uint8_t value) override {
            if (cartridge) {
                cartridge->ppuWrite(addr, value);
            }
        }
    };
}