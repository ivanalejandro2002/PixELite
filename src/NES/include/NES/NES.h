#pragma once
#include <NES/Device.h>

#include <NES/CPU/MOS6502.h>

#include <NES/Bus/Bus.h>

#include <NES/Memory/RAM.h>
#include <NES/Memory/PRGROM.h>

#include <NES/PPU/PPU2C02.h>
#include <NES/PPU/PPURegisters.h>

#include <NES/APU/APU.h>

#include <NES/Controller/Controller.h>
#include <NES/Cartridge/Cartridge.h>

namespace NES
{
    class NES
    {
        public:
            NES();

            void clock();

            void reset();

            void loadROM(const std::string &filepath);

            void mock();
            void memoryDebug();

            void debugNMI();
            void debugIRQ();
            void debugHighIRQ();
            void cpuWrite(uint16_t address, uint8_t value);
            uint8_t cpuRead(uint16_t address);

            bool isCpuJammed();

            const PPU2C02& getPPU() const;
            PPU2C02& getPPU();
            APU& getAPU();
            Controller& getController1();

            static std::vector<uint8_t> createTestProgram();
            static std::vector<uint8_t> createTestCHR();
        private:
            

            RAM ram;
            PRGROM prgRom;
            
            Bus mainBus;
            CPU::MOS6502 cpu;

            PPU2C02 ppu;
            PPURegisters ppuRegisters;

            APU apu;

            Controller controller1;

            Cartridge cartridge;

            uint64_t masterClock = 0;
    };
}