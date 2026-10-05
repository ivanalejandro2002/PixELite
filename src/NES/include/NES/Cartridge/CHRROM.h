#pragma once

#include <NES/Device.h>
#include <vector>
#include <cstdint>

namespace NES
{
    class CHRROM : public Device
    {
        public:
            explicit CHRROM(const std::vector<uint8_t> &data);
            
            bool handles(uint16_t addr) const override;

            uint8_t read(uint16_t addr) const override;
            void write(uint16_t addr, uint8_t value) override;

        private:
            std::vector<uint8_t> memory;
    };
}