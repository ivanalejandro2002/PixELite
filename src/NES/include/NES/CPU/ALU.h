#pragma once
#include <NES/CPU/ALUResult.h>

namespace NES::CPU
{
    class ALU
    {
        public:
            ALUResult add(uint8_t a, uint8_t b, bool carryIn);
            ALUResult substract(uint8_t a, uint8_t b, bool carryIn);
            ALUResult andOp(uint8_t a, uint8_t b);
            ALUResult orOp(uint8_t a, uint8_t b);
            ALUResult xorOp(uint8_t a, uint8_t b);
            ALUResult shiftLeft(uint8_t value);
            ALUResult shiftRight(uint8_t value);
            ALUResult rotateLeft(uint8_t value, bool carryIn);
            ALUResult rotateRight(uint8_t value, bool carryIn);
            ALUResult increment(uint8_t value);
            ALUResult decrement(uint8_t value);
            ALUResult compare(uint8_t a, uint8_t b);
            
        private:
    };
}