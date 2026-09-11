#include <NES/CPU/Instruction/InstructionBuilder.h>
#include <NES/CPU/Instruction/Instruction.h>

namespace NES::CPU
{
    InstructionBuilder& InstructionBuilder::add(MicroOperation operation)
    {
        operations.emplace_back(operation);
        return *this;
    }

    InstructionBuilder& InstructionBuilder::add(
        std::initializer_list<MicroOperation> mOperations
    )
    {
        operations.insert(
            operations.end(),
            mOperations.begin(),
            mOperations.end()
        );

        return *this;
    }

    Instruction InstructionBuilder::build(AddressingMode addressing, InstructionOperation operation, AccessType access)
    {
        return Instruction(addressing, operation, access, operations);
    }

    void InstructionBuilder::clear()
    {
        operations.clear();
    }
}