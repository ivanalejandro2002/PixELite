#pragma once

#include <NES/CPU/StatusRegister.h>
#include <NES/CPU/ALU.h>

namespace NES::CPU
{
    class MOS6502
    {
        public:
            MOS6502();

            void clock();
            void reset();

            uint64_t getCycles();

            void mock();
        private:
            uint8_t A;
            uint8_t X;
            uint8_t Y;
            uint16_t PC;
            uint8_t S;
            StatusRegister status;
            
            ALU alu;

            uint64_t cycles;
    };
}