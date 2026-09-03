#include <NES/CPU/Addressing/AddressingModes.h>
#include <NES/CPU/MOS6502.h>

namespace NES::CPU::AddressingModes
{
    void build(AddressingMode mode, InstructionBuilder& builder)
    {
        switch (mode)
        {
            case AddressingMode::Immediate:
                Immediate(builder);
                break;

            case AddressingMode::ZeroPage:
                ZeroPage(builder);
                break;

            case AddressingMode::ZeroPageX:
                ZeroPageX(builder);
                break;

            case AddressingMode::ZeroPageY:
                ZeroPageY(builder);
                break;

            case AddressingMode::Absolute:
                Absolute(builder);
                break;

            case AddressingMode::AbsoluteX:
                AbsoluteX(builder);
                break;

            case AddressingMode::AbsoluteY:
                AbsoluteY(builder);
                break;

            case AddressingMode::Indirect:
                Indirect(builder);
                break;

            case AddressingMode::IndirectIndexedX:
                IndirectIndexedX(builder);
                break;

            case AddressingMode::IndirectIndexedY:
                IndirectIndexedY(builder);
                break;

            case AddressingMode::Relative:
                Relative(builder);
                break;

            default:
                break;
        }
    }

    void Immediate(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchOperand);
    }

    void ZeroPage(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::ReadOperand);
    }

    void Absolute(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHigh)
            .add(MicroOperation::ReadOperand);

    }

    void ZeroPageX(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            //.add(MicroOperation::DummyRead);
            // Lógicamente aplica un Dummy Read, pero pasa siempre al sumar X en página 0.
            .add(MicroOperation::AddXToZeroPageAddress)
            .add(MicroOperation::ReadOperand);
    }

    void ZeroPageY(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            //.add(MicroOperation::DummyRead);
            // Lógicamente aplica un Dummy Read, pero pasa siempre al sumar X en página 0.
            .add(MicroOperation::AddYToZeroPageAddress)
            .add(MicroOperation::ReadOperand);
    }

    void AbsoluteX(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHigh)
            .add(MicroOperation::AddXToAddress)
            //Usado para verificar el uso de Dummy Read, si no se necesita ejecutar el siguiente en su lugar
            .add(MicroOperation::DummyRead);
            //.add(MicroOperation::ReadAbsoluteX);
    }

    void AbsoluteY(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHigh)
            .add(MicroOperation::AddYToAddress)
            //Usado para verificar el uso de Dummy Read, si no se necesita ejecutar el siguiente en su lugar
            .add(MicroOperation::DummyRead);
            //.add(MicroOperation::ReadAbsoluteX);
    }

    void Implied(InstructionBuilder& builder)
    {
        
    }

    void Accumulator(InstructionBuilder& builder)
    {
        // El operando está en A.
    }

    void Indirect(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHigh)
            .add(MicroOperation::FetchIndirectLow)
            // Puede existir el bug de cambio de página
            .add(MicroOperation::FetchIndirectHigh);

    }

    void IndirectIndexedX(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddXToZeroPageAddress)
            .add(MicroOperation::FetchIndirectLow)
            //Puede existir el bug de cambio de página
            .add(MicroOperation::FetchIndirectHigh)
            .add(MicroOperation::ReadOperand);
    }

    void IndirectIndexedY(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::FetchIndirectLow)
            //Puede existir el bug de cambio de página
            .add(MicroOperation::FetchIndirectHigh)
            .add(MicroOperation::AddYToAddress)
            //Usado para verificar el uso de Dummy Read, si no se necesita ejecutar el siguiente en su lugar
            .add(MicroOperation::DummyRead);
            // .add(MicroOperation::ReadOperand);
    }

    void Relative(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::EvaluateBranch)
            .add(MicroOperation::DummyRead);
    }
}