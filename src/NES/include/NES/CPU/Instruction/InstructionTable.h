#pragma once

#include <array>
#include <cstdint>

#include <NES/CPU/Instruction/InstructionDefinition.h>

namespace NES::CPU
{
    class InstructionTable
    {
        public:
            InstructionTable();

            const InstructionDefinition& get(uint8_t opcode)const;
        private:
            std::array<InstructionDefinition, 256> table;

            void set(
                uint8_t opcode,
                InstructionOperation operation,
                AddressingMode addressingMode,
                uint8_t cycles
            );

            void buildTable();
    };
}