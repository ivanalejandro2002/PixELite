#pragma once
#include <vector>
#include <NES/CPU/Instruction/MicroOperation.h>
#include <NES/CPU/Instruction/InstructionOperation.h>
#include <NES/CPU/Addressing/AddressingMode.h>

namespace NES::CPU
{
    class Instruction;

    class InstructionBuilder
    {
        public:
            InstructionBuilder &add(MicroOperation operation);
            InstructionBuilder &add(std::initializer_list<MicroOperation> mOperations);

            Instruction build(AddressingMode addresing, InstructionOperation operation, AccessType access);

            void clear();
        private:
            std::vector<MicroOperation> operations;
    };
}