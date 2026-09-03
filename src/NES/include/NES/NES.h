#pragma once
#include <NES/Device.h>

#include <NES/CPU/MOS6502.h>

#include <NES/Bus/Bus.h>

#include <NES/Memory/RAM.h>
#include <NES/Memory/PRGROM.h>

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
            static std::vector<uint8_t> createTestProgram();

            RAM ram;
            PRGROM prgRom;
            
            Bus mainBus;
            CPU::MOS6502 cpu;

            uint64_t masterClock = 0;
    };
}