#pragma once
#include <cstdint>

namespace NES
{
    class Device
    {
        public:
            virtual ~Device() = default;

            virtual bool handles(uint16_t addr)const = 0;
            virtual uint8_t read(uint16_t addr) = 0;
            virtual void write(uint16_t addr, uint8_t value) = 0;

            bool isWritable = 0;
    };
}