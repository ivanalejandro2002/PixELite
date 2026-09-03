#pragma once
#include <NES/Device.h>

#include <vector>
#include <cstdint>

namespace NES
{
    class PRGROM : public Device
    {
        public:
            explicit PRGROM(
                const std::vector<uint8_t>& data
            );

            bool handles(uint16_t addr) const override;

            uint8_t read(uint16_t addr) override;

            void write(
                uint16_t addr,
                uint8_t value
            ) override;

        private:
            std::vector<uint8_t> memory;
    };
}