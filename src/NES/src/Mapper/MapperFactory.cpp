#include <NES/Mapper/MapperFactory.h>
#include <NES/Mapper/MapperIncludes.h>
#include <iostream>

namespace NES {
    std::unique_ptr<Mapper> MapperFactory::createMapper(uint16_t mapperNumber, uint8_t prgBanks, uint8_t chrBanks) {
        switch (mapperNumber) {
            case 0:
                return std::make_unique<Mapper0>(prgBanks, chrBanks);
            case 1:
                // return std::make_unique<Mapper1>(prgBanks, chrBanks);
                break;
            case 2:
                return std::make_unique<Mapper2>(prgBanks, chrBanks);
            default:
                std::cerr << "[MapperFactory] Error: Mapper " << mapperNumber << " no soportado o no implementado.\n";
                return nullptr;
        }
        return nullptr;
    }
}