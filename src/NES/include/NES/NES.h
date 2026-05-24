#pragma once
#include <NES/Device.h>

#include <NES/CPU/MOS6502.h>

#include <NES/Bus/Bus.h>

#include <NES/Memory/RAM.h>

namespace NES
{
    class NES
    {
        public:
            NES();

            void clock();

            void reset();

            void mock();
        private:
            Bus mainBus;
            CPU::MOS6502 cpu;
            RAM ram;

            uint64_t masterClock = 0;
    };
}