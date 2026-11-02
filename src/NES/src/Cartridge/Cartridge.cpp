#include <NES/Cartridge/Cartridge.h>

namespace NES
{
    Cartridge::Cartridge(){};

    bool Cartridge::loadFromFile(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::binary);
        if(!file.is_open())
        {
            std::cerr << "No se pudo abrir el archivo de ROM: " << filepath << "\n";
            return false;
        }

        RomHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(RomHeader));

        if(header.magic[0] != 'N' || header.magic[1] != 'E' ||
            header.magic[2] != 'S' || header.magic[3] != 0x1A)
        {
            std::cerr << "Error: Formato ROM inválido (Número mágico NES incorrecto)\n";
            return false;
        }

        uint8_t Nes2Check = header.flags7 & 0x0C;
        if(Nes2Check == 0x08) isNes20 = true;
        else isNes20 = false;

        if(isNes20)
        {
            uint16_t mapperLow = (header.flags6 >> 4) & 0x0F;
            uint16_t mapperHigh = (header.flags7 & 0xF0);
            uint16_t mapperUpper = (header.flags8 & 0x0F) << 8;
            mapperNumber = mapperLow | mapperHigh | mapperUpper;

            subMapperNumber = (header.flags8 >> 4) & 0x0F;
        }
        else
        {
            mapperNumber = (header.flags7 & 0xF0) | ((header.flags6 >> 4) & 0x0F);
            subMapperNumber = 0;
        }

        bool hardWiredFourScreen = (header.flags6 & 0x08) != 0;
        hasBattery = (header.flags6 & 0x02) != 0;
        hasTrainer = (header.flags6 & 0x04) != 0;

        if(hardWiredFourScreen) mirroring = Mirroring::FourScreen;
        else if(header.flags6 & 0x01) mirroring = Mirroring::Vertical;
        else mirroring = Mirroring::Horizontal;

        size_t prgSize = 0;
        size_t chrSize = 0;

        if(isNes20)
        {
            uint16_t prgBlocks = header.prgSizeLsb | ((header.flags9 & 0x0F) << 8);
            uint16_t chrBlocks = header.chrSizeLsb | ((header.flags9 & 0xF0) << 4);

            prgSize = prgBlocks * 16384;
            chrSize = chrBlocks * 8192;
        }
        else
        {
            prgSize = header.prgSizeLsb * 18384;
            chrSize = header.chrSizeLsb * 8192;
        }

        size_t prgRamSize = 0;
        size_t prgNvRamSize = 0;
        size_t chrRamSize = 0;
        size_t chrNvRamSize = 0;

        if(isNes20)
        {
            prgRamSize = decodeRamSize(header.flags10 & 0x0F);
            prgNvRamSize = decodeRamSize((header.flags10 >> 4) & 0x0F);
            chrRamSize = decodeRamSize(header.flags11 & 0x0F);
            chrNvRamSize = decodeRamSize((header.flags11 >> 4) & 0x0F);

            if(prgNvRamSize > 0) hasBattery = true;

            uint8_t timingMode = header.flags12 & 0x03;
            switch(timingMode)
            {
                case 0:
                    timing = ConsoleTiming::NTSC;
                    break;
                case 1:
                    timing = ConsoleTiming::PAL;
                    break;
                case 2:
                    timing = ConsoleTiming::Multiple;
                    break;
                case 3:
                    timing = ConsoleTiming::Dendy;
                    break;
            }
        }
        else
        {
            if(hasBattery) prgNvRamSize = 8192;
        }

        if(hasTrainer)
        {
            trainer.resize(512);
            file.read(reinterpret_cast<char*>(trainer.data()), 512);
        }

        if(prgSize > 0)
        {
            prgRom.resize(prgSize);
            file.read(reinterpret_cast<char*>(prgRom.data()), prgSize);
        }
        else
        {
            std::cerr << "Error crítico: El archivo carece de datos PRG-ROM válidos\n";
            return false;
        }

        if(chrSize > 0)
        {
            chrRom.resize(chrSize);
            file.read(reinterpret_cast<char*>(chrRom.data()), chrSize);
        }
        else
        {
            size_t allocatedChrRam = (chrRamSize > 0) ? chrRamSize : 8192;
            chrRom.resize(allocatedChrRam, 0);
        }

        std::cout << "[Cartridge] Archivo analizado correctamente:\n"
                    << "  - Estándar detectado: " << (isNes20 ? "NES 2.0" : "iNES Clásico") << "\n"
                    << "  - Mapper ID: " << mapperNumber << (isNes20 ? " (Submapper: " + std::to_string(subMapperNumber) + ")" : "") << "\n"
                    << "  - PRG-ROM Size: " << prgSize / 1024 << " KB\n"
                    << "  - CHR Size: " << (chrSize > 0 ? std::to_string(chrSize / 1024) + " KB (ROM)" : std::to_string(chrRom.size() / 1024) + " KB (RAM)") << "\n"
                    << "  - PRG-RAM / NVRAM: " << (prgRamSize + prgNvRamSize) << " bytes (Batería: " << (hasBattery ? "Sí" : "No") << ")\n"
                    << "  - Trainer: " << (hasTrainer ? "Presente (512 bytes)" : "Ausente") << "\n"
                    << "  - Mirroring: " << (mirroring == Mirroring::Vertical ? "Vertical" : (mirroring == Mirroring::Horizontal ? "Horizontal" : "4-Screen")) << "\n"
                    << (isNes20 ? "  - Región de Hardware: " + std::string(timing == ConsoleTiming::NTSC ? "NTSC" : timing == ConsoleTiming::PAL ? "PAL" : "Dendy/Multiple") + "\n" : "");

        return true;
    }

    size_t Cartridge::decodeRamSize(uint8_t shiftCount) const
    {
        if(shiftCount == 0) return 0;
        return static_cast<size_t>(64) << shiftCount;
    }
}