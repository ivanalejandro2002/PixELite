#pragma once

#include <NES/CPU/Addressing/AddressingMode.h>
#include <NES/CPU/Instruction/InstructionBuilder.h>

namespace NES::CPU
{
    class InstructionBuilder;
    namespace AddressingModes
    {
        void build(AddressingMode mode, InstructionBuilder& builder);
        void buildRead(AddressingMode mode, InstructionBuilder& builder);
        void buildWrite(AddressingMode mode, InstructionBuilder& builder);
        void buildRMW(AddressingMode mode, InstructionBuilder& builder);

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



        void ImpliedRead(InstructionBuilder&);

        void ImmediateRead(InstructionBuilder&);

        void ZeroPageRead(InstructionBuilder&);
        void ZeroPageXRead(InstructionBuilder&);
        void ZeroPageYRead(InstructionBuilder&);

        void AbsoluteRead(InstructionBuilder&);
        void AbsoluteXRead(InstructionBuilder&);
        void AbsoluteYRead(InstructionBuilder&);

        void IndirectRead(InstructionBuilder&);
        
        void IndirectIndexedXRead(InstructionBuilder&);
        void IndirectIndexedYRead(InstructionBuilder&);



        void ImpliedWrite(InstructionBuilder&);

        void ImmediateWrite(InstructionBuilder&);

        void ZeroPageWrite(InstructionBuilder&);
        void ZeroPageXWrite(InstructionBuilder&);
        void ZeroPageYWrite(InstructionBuilder&);

        void AbsoluteWrite(InstructionBuilder&);
        void AbsoluteXWrite(InstructionBuilder&);
        void AbsoluteYWrite(InstructionBuilder&);
        
        void IndirectIndexedXWrite(InstructionBuilder&);
        void IndirectIndexedYWrite(InstructionBuilder&);



        void ImpliedRMW(InstructionBuilder&);

        void ImmediateRMW(InstructionBuilder&);

        void ZeroPageRMW(InstructionBuilder&);
        void ZeroPageXRMW(InstructionBuilder&);
        void ZeroPageYRMW(InstructionBuilder&);

        void AbsoluteRMW(InstructionBuilder&);
        void AbsoluteXRMW(InstructionBuilder&);
        void AbsoluteYRMW(InstructionBuilder&);
        
        void IndirectIndexedXRMW(InstructionBuilder&);
        void IndirectIndexedYRMW(InstructionBuilder&);
    }
}