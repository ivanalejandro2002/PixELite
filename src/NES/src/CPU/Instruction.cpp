#include <NES/CPU/Instruction/Instruction.h>
#include <NES/CPU/MOS6502.h>

namespace NES::CPU
{
    Instruction::Instruction(
        InstructionOperation operation,
        const std::vector<MicroOperation>& operations
    ):operation(operation),operations(operations),currentOperation(0){}

    void Instruction::tick(MOS6502& cpu)
    {
        if(finished())
            return;

        cpu.execute(operations[currentOperation]);
        currentOperation++;
    }

    bool Instruction::finished() const
    {
        return currentOperation >= operations.size();
    }

    void Instruction::reset()
    {
        currentOperation = 0;
    }

    InstructionOperation Instruction::getOperation() const
    {
        return operation;
    }

    void Instruction::printInstruction()
    {
        std::cout << "Operación: " << InstructionOperationMapper::ToString(operation) << "\n";
        std::cout << "Size: " << operations.size() << "\n";
        std::cout << "Elements: ";
        for(auto &e: operations) std::cout << MicroOperationMapper::ToString(e) << ", ";
        std::cout << "\n";
    }
}