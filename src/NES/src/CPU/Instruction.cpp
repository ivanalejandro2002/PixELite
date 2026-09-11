#include <NES/CPU/Instruction/Instruction.h>
#include <NES/CPU/MOS6502.h>

namespace NES::CPU
{
    Instruction::Instruction(
        AddressingMode addressing,
        InstructionOperation operation,
        AccessType access,
        const std::vector<MicroOperation>& operations
    ):addressingMode(addressing),operation(operation),accessType(access),operations(operations),currentOperation(0){}

    void Instruction::tick(MOS6502& cpu)
    {
        if(finished())
            return;

        MicroOperation operation = operations[currentOperation];
        ++currentOperation;

        cpu.execute(operation);
    }

    bool Instruction::finished() const
    {
        return currentOperation >= operations.size();
    }
    
    void Instruction::endTask()
    {
        currentOperation = operations.size();
    }

    void Instruction::insertNext(MicroOperation operation)
    {
        operations.insert(
            operations.begin() + currentOperation,
            operation
        );
    }

    void Instruction::insertNextSequence(
        std::initializer_list<MicroOperation> operationList
    )
    {
        operations.insert(
            operations.begin() + currentOperation,
            operationList.begin(),
            operationList.end()
        );
    }

    void Instruction::reset()
    {
        currentOperation = 0;
    }

    InstructionOperation Instruction::getOperation() const
    {
        return operation;
    }

    AddressingMode Instruction::getAddressingMode() const
    {
        return addressingMode;
    }

    AccessType Instruction::getAccessType() const
    {
        return accessType;
    }

    void Instruction::printInstruction()
    {
        std::cout << "Addressing Mode: " << AddressingModeMapper::ToString(addressingMode) << "\n";
        std::cout << "Operación: " << InstructionOperationMapper::ToString(operation) << "\n";
        std::cout << "Access Type: " << AccessTypeMapper::ToString(accessType) << "\n";
        std::cout << "Size: " << operations.size() << "\n";
        std::cout << "Elements: ";
        for(auto &e: operations) std::cout << MicroOperationMapper::ToString(e) << ", ";
        std::cout << "\n";
    }
}