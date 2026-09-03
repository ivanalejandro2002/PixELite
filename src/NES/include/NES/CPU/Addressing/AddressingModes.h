#pragma once

#include <NES/CPU/Addressing/AddressingMode.h>
#include <NES/CPU/Instruction/InstructionBuilder.h>

namespace NES::CPU
{
    class InstructionBuilder;
    namespace AddressingModes
    {
        void build(AddressingMode mode, InstructionBuilder& builder);

        void Implied(InstructionBuilder&);

        void Accumulator(InstructionBuilder&);

        void Immediate(InstructionBuilder&);

        void ZeroPage(InstructionBuilder&);
        void ZeroPageX(InstructionBuilder&);
        void ZeroPageY(InstructionBuilder&);

        void Absolute(InstructionBuilder&);
        void AbsoluteX(InstructionBuilder&);
        void AbsoluteY(InstructionBuilder&);

        void Indirect(InstructionBuilder&);
        
        void IndirectIndexedX(InstructionBuilder&);
        void IndirectIndexedY(InstructionBuilder&);

        void Relative(InstructionBuilder&);
    }
}