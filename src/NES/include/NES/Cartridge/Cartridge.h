#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <cmath>

#include <NES/PPU/Mirroring.h>

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

    class Cartridge
    {
        public:
            Cartridge();
            bool loadFromFile(const std::string& filepath);

            const std::vector<uint8_t>& getPRG() const;
            const std::vector<uint8_t>& getCHR() const;
            const std::vector<uint8_t>& getTrainer() const;
            uint16_t getMapperNumber() const;
            uint8_t getSubmapperNumber() const;
            Mirroring getMirroring() const;
            bool hasBatteryBackedRAM() const;
            bool hasActiveTrainer() const;

        private:
            std::vector<uint8_t> prgRom;
            std::vector<uint8_t> chrRom;
            std::vector<uint8_t> trainer;
            std::vector<uint8_t> prgRam;

            uint16_t mapperNumber = 0;
            uint8_t subMapperNumber = 0;
            Mirroring mirroring = Mirroring::Horizontal;
            ConsoleTiming timing = ConsoleTiming::NTSC;

            bool hasBattery = false;
            bool hasTrainer = false;
            bool isNes20 = false;

            size_t decodeRamSize(uint8_t shiftCount) const;
    };
}