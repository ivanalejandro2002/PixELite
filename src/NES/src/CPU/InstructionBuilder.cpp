#include <NES/CPU/Instruction/InstructionBuilder.h>
#include <NES/CPU/Instruction/Instruction.h>

namespace NES::CPU
{
    InstructionBuilder& InstructionBuilder::add(MicroOperation operation)
    {
        operations.emplace_back(operation);
        return *this;
    }

    Instruction InstructionBuilder::build(InstructionOperation operation)
    {
        return Instruction(operation, operations);
    }

    void InstructionBuilder::clear()
    {
        operations.clear();
    }
}