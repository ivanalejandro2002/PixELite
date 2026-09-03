#pragma once
#include <vector>
#include <NES/CPU/Instruction/MicroOperation.h>
#include <NES/CPU/Instruction/InstructionOperation.h>

namespace NES::CPU
{
    class Instruction;

    class InstructionBuilder
    {
        public:
            InstructionBuilder &add(MicroOperation operation);

            Instruction build(InstructionOperation operation);

            void clear();
        private:
            std::vector<MicroOperation> operations;
    };
}