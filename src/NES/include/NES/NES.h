#pragma once
#include <NES/Device.h>

#include <NES/CPU/MOS6502.h>

#include <NES/Bus/Bus.h>

#include <NES/Memory/RAM.h>
#include <NES/Memory/PRGROM.h>

#include <NES/PPU/PPU2C02.h>
#include <NES/PPU/PPURegisters.h>

namespace NES
{
    class NES
    {
        public:
            NES();

            void clock();

            void reset();

            void mock();
            void memoryDebug();

            void debugNMI();
            void debugIRQ();
            void debugHighIRQ();

            bool isCpuJammed();

            const PPU2C02& getPPU() const;
            PPU2C02& getPPU();

            static std::vector<uint8_t> createTestProgram();
            static std::vector<uint8_t> createTestCHR();
        private:
            

            RAM ram;
            PRGROM prgRom;
            
            Bus mainBus;
            CPU::MOS6502 cpu;

            PPU2C02 ppu;
            PPURegisters ppuRegisters;

            uint64_t masterClock = 0;
    };
}