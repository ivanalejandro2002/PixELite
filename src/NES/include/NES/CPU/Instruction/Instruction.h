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
            
            void skipNextOperation() {
                if (currentOperation < operations.size()) {
                    ++currentOperation;
                }
            }

            const std::vector<MicroOperation>& getOperationsList() const { return operations; }
            std::size_t getCurrentOperationIndex() const { return currentOperation; }

            void restoreState(const std::vector<MicroOperation>& ops, std::size_t currentIdx) {
                operations = ops;
                currentOperation = currentIdx;
            }

            InstructionOperation getOperation() const;
            AddressingMode getAddressingMode() const;
            AccessType getAccessType() const;
            void printInstruction();
            uint8_t getRemainingOperations() const;
            MicroOperation getBack() const;
            bool pollable() const;

            bool isCurrentOperationWrite() const;
            MicroOperation getCurrentOperationType() const;

        private:
            AddressingMode addressingMode;
            InstructionOperation operation;
            AccessType accessType;
            std::vector<MicroOperation> operations;
            std::size_t currentOperation = 0;
    };
}