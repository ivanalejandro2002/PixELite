#pragma once

#include <NES/CPU/Instruction/Instruction.h>
#include <NES/CPU/Instruction/InstructionDefinition.h>
#include <NES/CPU/Instruction/InstructionBuilder.h>
#include <NES/CPU/Addressing/AddressingModes.h>

namespace NES::CPU
{
    class InstructionFactory
    {
        public:
            InstructionFactory() = default;

            Instruction create(
                const InstructionDefinition& definition
            ) const;

        
        private:

            Instruction buildJSR() const;
            Instruction buildRTS() const;
            Instruction buildBRK() const;
            Instruction buildRTI() const;

            Instruction buildPHA() const;
            Instruction buildPLA() const;
            Instruction buildPHP() const;
            Instruction buildPLP() const;
    };
}