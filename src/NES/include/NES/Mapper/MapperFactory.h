#pragma once
#include <memory>
#include <cstdint>
#include "Mapper.h"

namespace NES {
    class MapperFactory {
    public:
        static std::unique_ptr<Mapper> createMapper(uint16_t mapperNumber, uint8_t prgBanks, uint8_t chrBanks);
    };
}