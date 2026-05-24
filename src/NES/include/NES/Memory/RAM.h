#pragma once
#include <NES/Device.h>

#include <array>

namespace NES
{
    class RAM: public Device
    {
        public:
            bool handles(uint16_t addr)const override;
            
            uint8_t read(uint16_t addr)override;
            void write(uint16_t addr, uint8_t value)override;
        
        private:
            std::array<uint8_t, 2048> memory;
    };
}