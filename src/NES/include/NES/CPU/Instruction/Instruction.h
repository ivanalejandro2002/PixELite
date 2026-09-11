#pragma once
#include <vector>
#include <cstddef>
#include <iostream>

#include <NES/CPU/Instruction/MicroOperation.h>
#include <NES/CPU/Instruction/InstructionOperation.h>
#include <NES/CPU/Addressing/AddressingMode.h>

namespace NES::CPU
{
    class MOS6502;
    
    class Instruction
    {
        public:
            Instruction(
                AddressingMode addressing,
                InstructionOperation operation,
                AccessType access,
                const std::vector<MicroOperation>& operations
            );
            void tick(MOS6502& cpu);
            bool finished() const;
            void endTask();
            void reset();
            void insertNext(MicroOperation operation);
            void insertNextSequence(std::initializer_list<MicroOperation> operationList);

            InstructionOperation getOperation() const;
            AddressingMode getAddressingMode() const;
            AccessType getAccessType() const;
            void printInstruction();

            private:
            AddressingMode addressingMode;
            InstructionOperation operation;
            AccessType accessType;
            std::vector<MicroOperation> operations;
            std::size_t currentOperation = 0;
    };
}