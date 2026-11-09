#include <NES/Cartridge/Cartridge.h>
#include <NES/Mapper/MapperFactory.h>

#include <fstream>
#include <filesystem>

namespace NES
{
    Cartridge::Cartridge(){};

    bool Cartridge::handlesRead(uint16_t addr) const {
        // El cartucho responde al rango de PRG del CPU (y opcionalmente RAM de batería $6000-$7FFF)
        return (addr >= 0x8000 && addr <= 0xFFFF) ||
                (addr >= 0x6000 && addr <= 0x7FFF);
    }

    bool Cartridge::handlesWrite(uint16_t addr) const {
        // El cartucho responde al rango de PRG del CPU (y opcionalmente RAM de batería $6000-$7FFF)
        return (addr >= 0x8000 && addr <= 0xFFFF) ||
                (addr >= 0x6000 && addr <= 0x7FFF);
    }

    uint8_t Cartridge::read(uint16_t addr) {
        if (addr >= 0x6000 && addr <= 0x7FFF) {
            // Leer de la PRG-RAM del cartucho (ajustando la dirección base a 0)
            if (!prgRam.empty()) {
                return prgRam[addr - 0x6000];
            }
            return 0;
        }
        
        if (addr >= 0x8000 && addr <= 0xFFFF) {
            uint32_t mappedAddr = 0;
            if (mapper && mapper->cpuMapRead(addr, mappedAddr)) {
                if (mappedAddr < prgRom.size()) {
                    return prgRom[mappedAddr];
                }
            }
        }
        return 0;
    }

    void Cartridge::write(uint16_t addr, uint8_t value) {
        if (addr >= 0x6000 && addr <= 0x7FFF) {
            // Escribir en la PRG-RAM del cartucho
            if (!prgRam.empty()) {
                prgRam[addr - 0x6000] = value;
            }
            return;
        }

        if (addr >= 0x8000 && addr <= 0xFFFF) {
            uint32_t mappedAddr = 0;
            if (mapper) {
                // El mapper procesa la escritura (cambio de banco o registros del MMC1)
                mapper->cpuMapWrite(addr, mappedAddr, value);
            }
        }
    }

    // Métodos para que la PPU también consulte la CHR a través del Mapper/Cartucho
    uint8_t Cartridge::ppuRead(uint16_t addr) {
        uint32_t mappedAddr = 0;
        if (mapper && mapper->ppuMapRead(addr, mappedAddr)) {
            if (mappedAddr < chrRom.size()) {
                return chrRom[mappedAddr];
            }
        }
        return 0;
    }

    void Cartridge::ppuWrite(uint16_t addr, uint8_t value) {
        uint32_t mappedAddr = 0;
        if (mapper && mapper->ppuMapWrite(addr, mappedAddr)) {
            if (mappedAddr < chrRom.size()) {
                chrRom[mappedAddr] = value;
            }
        }
    }

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
            prgSize = header.prgSizeLsb * 16384;
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

            std::filesystem::path rootPath(PROJECT_ROOT_DIR);

            std::filesystem::path realPath = rootPath / "debugs" / "PRGROM.out";

            std::filesystem::create_directories(realPath.parent_path()); 
            
            std::ofstream fout(realPath);
            for(int i = 0; i < prgSize; ++i)
            {
                if(!(i & 0x00FF))fout << "\n" << ((i & 0xFF00) >> 8) << ":\n";

                if(!(i & 0x000F))fout << "\n" << ((i & 0x00F0) >> 4) << ": ";
                fout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                << static_cast<int>(prgRom[i]) << " "; 
            }
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

        size_t totalPrgRamSize = prgRamSize + prgNvRamSize;

        if (totalPrgRamSize == 0 && mapperNumber == 1) {
            totalPrgRamSize = 8192; // 8 KB estándar de trabajo
        }

        if (totalPrgRamSize > 0) {
            prgRam.resize(totalPrgRamSize, 0);
        }

        std::cout << "[Cartridge] Archivo analizado correctamente:\n"
                    << "  - Estándar detectado: " << (isNes20 ? "NES 2.0" : "iNES Clásico") << "\n"
                    << "  - Mapper ID: " << mapperNumber << (isNes20 ? " (Submapper: " + std::to_string(subMapperNumber) + ")" : "") << "\n"
                    << "  - PRG-ROM Size: " << prgSize / 1024 << " KB\n"
                    << "  - CHR Size: " << (chrSize > 0 ? std::to_string(chrSize / 1024) + " KB (ROM)" : std::to_string(chrRom.size() / 1024) + " KB (RAM)") << "\n"
                    << "  - PRG-RAM / NVRAM: " << (totalPrgRamSize) << " bytes (Batería: " << (hasBattery ? "Sí" : "No") << ")\n"
                    << "  - Trainer: " << (hasTrainer ? "Presente (512 bytes)" : "Ausente") << "\n"
                    << "  - Mirroring: " << (mirroring == Mirroring::Vertical ? "Vertical" : (mirroring == Mirroring::Horizontal ? "Horizontal" : "4-Screen")) << "\n"
                    << (isNes20 ? "  - Región de Hardware: " + std::string(timing == ConsoleTiming::NTSC ? "NTSC" : timing == ConsoleTiming::PAL ? "PAL" : "Dendy/Multiple") + "\n" : "");

        uint8_t prgBanks = static_cast<uint8_t>(prgSize / 16384);
        uint8_t chrBanks = static_cast<uint8_t>(chrRom.size() / 8192);

        mapper = MapperFactory::createMapper(mapperNumber, prgBanks, chrBanks);

        if (!mapper) {
            std::cerr << "[Cartridge] Error crítico: No se pudo instanciar el Mapper ID " << mapperNumber << "\n";
            return false;
        }

        std::cout << "[Cartridge] ROM cargada con éxito. Mapper " << mapperNumber << " inicializado correctamente.\n";

        return true;
    }

    size_t Cartridge::decodeRamSize(uint8_t shiftCount) const
    {
        if(shiftCount == 0) return 0;
        return static_cast<size_t>(64) << shiftCount;
    }

    Mirroring Cartridge::getActiveMirroring() const {
        if (mapper) {
            return mapper->getMapperMirroring(); // Llama al método exclusivo del mapper
        }
        return mirroring; // Valor por defecto del header iNES
    }
}