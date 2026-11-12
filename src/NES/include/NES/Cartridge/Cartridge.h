#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <cmath>
#include <memory>

#include <NES/PPU/Mirroring.h>
#include <NES/Device.h>
#include <NES/Mapper/MapperIncludes.h>

namespace NES{
    struct RomHeader
    {
        char magic[4];
        uint8_t prgSizeLsb;
        uint8_t chrSizeLsb;
        uint8_t flags6;
        uint8_t flags7;
        uint8_t flags8;
        uint8_t flags9;
        uint8_t flags10;
        uint8_t flags11;
        uint8_t flags12;
        uint8_t flags13;
        uint8_t flags14;
        uint8_t flags15;
    };

    enum class ConsoleTiming{
        NTSC = 0,
        PAL = 1,
        Multiple = 2,
        Dendy = 3
    };

    class Cartridge : public Device
    {
        public:
            Cartridge();
            
            void resetMapper() { if (mapper) mapper->reset(); }

            bool handlesRead(uint16_t addr) const override;
            bool handlesWrite(uint16_t addr) const override;
            
            uint8_t read(uint16_t addr) override;
            void write(uint16_t addr, uint8_t data) override;
            uint8_t ppuRead(uint16_t addr);
            void ppuWrite(uint16_t addr, uint8_t value);

            bool loadFromFile(const std::string& filepath);

            const std::vector<uint8_t>& getPRG() const {return prgRom;}
            const std::vector<uint8_t>& getCHR() const {return chrRom;}
            const std::vector<uint8_t>& getTrainer() const {return trainer;};
            uint16_t getMapperNumber() const {return mapperNumber;}
            uint8_t getSubmapperNumber() const {return subMapperNumber;}
            Mirroring getMirroring() const {return mirroring;}
            bool hasBatteryBackedRAM() const {return hasBattery;}
            bool hasActiveTrainer() const {return hasTrainer;}

            Mirroring getActiveMirroring() const;

            bool irqState() const { return mapper ? mapper->irqState() : false; }
            void notifyPpuAddress(uint16_t addr) { if (mapper) mapper->notifyPpuaddress(addr); }

            bool hasPendingIRQ() const override { return mapper ? mapper->irqState() : false; }

            void saveState(std::ostream& out) const;
            void loadState(std::istream& in);

        private:
            std::vector<uint8_t> prgRom;
            std::vector<uint8_t> chrRom;
            std::vector<uint8_t> trainer;
            std::vector<uint8_t> prgRam;
            std::unique_ptr<Mapper> mapper;

            uint16_t mapperNumber = 0;
            uint8_t subMapperNumber = 0;
            Mirroring mirroring = Mirroring::Horizontal;
            ConsoleTiming timing = ConsoleTiming::NTSC;

            bool hasBattery = false;
            bool hasTrainer = false;
            bool isNes20 = false;

            bool usesChrRam = false;

            size_t decodeRamSize(uint8_t shiftCount) const;
    };
}