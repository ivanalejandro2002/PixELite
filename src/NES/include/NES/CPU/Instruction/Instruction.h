#pragma once
#include <vector>
#include <cstddef>
#include <iostream>

#include <NES/CPU/Instruction/MicroOperation.h>
#include <NES/CPU/Instruction/InstructionOperation.h>

namespace NES::CPU
{
    class MOS6502;
    
    class Instruction
    {
        public:
            Instruction(InstructionOperation operation,const std::vector<MicroOperation>& operations);
            void tick(MOS6502& cpu);
            bool finished() const;
            void reset();

            InstructionOperation getOperation() const;
            void printInstruction();
        private:
            InstructionOperation operation;
            std::vector<MicroOperation> operations;
            std::size_t currentOperation = 0;
    };
}