#pragma once

#include <cstdint>

#include <NES/CPU/Instruction/InstructionOperation.h>
#include <NES/CPU/Addressing/AddressingMode.h>

namespace NES::CPU
{
    struct InstructionDefinition
    {
        InstructionOperation operation;
        AddressingMode addressingMode;
        AccessType accessType;

        uint8_t cycles;
    };
}