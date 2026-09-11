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
            
            case AddressingMode::Accumulator:
                Accumulator(builder);

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

    void buildRead(AddressingMode mode, InstructionBuilder& builder)
    {
        switch (mode)
        {
            case AddressingMode::Immediate:
                ImmediateRead(builder);
                break;

            case AddressingMode::ZeroPage:
                ZeroPageRead(builder);
                break;

            case AddressingMode::ZeroPageX:
                ZeroPageXRead(builder);
                break;

            case AddressingMode::ZeroPageY:
                ZeroPageYRead(builder);
                break;

            case AddressingMode::Absolute:
                AbsoluteRead(builder);
                break;

            case AddressingMode::AbsoluteX:
                AbsoluteXRead(builder);
                break;

            case AddressingMode::AbsoluteY:
                AbsoluteYRead(builder);
                break;

            case AddressingMode::Indirect:
                IndirectRead(builder);
                break;

            case AddressingMode::IndirectIndexedX:
                IndirectIndexedXRead(builder);
                break;

            case AddressingMode::IndirectIndexedY:
                IndirectIndexedYRead(builder);
                break;

            default:
                std::cout << "Error, no debería llegarse a Lectura con el modo " << AddressingModeMapper::ToString(mode) << "\n";
                break;
        }
    }

    void buildWrite(AddressingMode mode, InstructionBuilder& builder)
    {
        switch (mode)
        {

            case AddressingMode::ZeroPage:
                ZeroPageWrite(builder);
                break;

            case AddressingMode::ZeroPageX:
                ZeroPageXWrite(builder);
                break;

            case AddressingMode::ZeroPageY:
                ZeroPageYWrite(builder);
                break;

            case AddressingMode::Absolute:
                AbsoluteWrite(builder);
                break;

            case AddressingMode::AbsoluteX:
                AbsoluteXWrite(builder);
                break;

            case AddressingMode::AbsoluteY:
                AbsoluteYWrite(builder);
                break;

            case AddressingMode::IndirectIndexedX:
                IndirectIndexedXWrite(builder);
                break;

            case AddressingMode::IndirectIndexedY:
                IndirectIndexedYWrite(builder);
                break;

            default:
                std::cout << "Error, no debería llegarse a Escritura con el modo " << AddressingModeMapper::ToString(mode) << "\n";
                break;
        }
    }

    void buildRMW(AddressingMode mode, InstructionBuilder& builder)
    {
        switch (mode)
        {

            case AddressingMode::ZeroPage:
                ZeroPageRMW(builder);
                break;

            case AddressingMode::ZeroPageX:
                ZeroPageXRMW(builder);
                break;

            case AddressingMode::ZeroPageY:
                ZeroPageYRMW(builder);
                break;

            case AddressingMode::Absolute:
                AbsoluteRMW(builder);
                break;

            case AddressingMode::AbsoluteX:
                AbsoluteXRMW(builder);
                break;

            case AddressingMode::AbsoluteY:
                AbsoluteYRMW(builder);
                break;

            case AddressingMode::IndirectIndexedX:
                IndirectIndexedXRMW(builder);
                break;

            case AddressingMode::IndirectIndexedY:
                IndirectIndexedYRMW(builder);
                break;

            default:
                std::cout << "Error, no debería llegarse a RMW con el modo " << AddressingModeMapper::ToString(mode) << "\n";
                break;
        }
    }

    void Immediate(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchOperand);
    }

    void ImmediateRead(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchOperand);
    }

    void ZeroPage(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::ReadOperandAndExecute);
    }

    void ZeroPageRead(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::ReadOperandAndExecute);
    }

    void ZeroPageWrite(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::WriteOperand);
    }

    void ZeroPageRMW(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::ReadOperand)
            .add(MicroOperation::DummyWrite)
            .add(MicroOperation::WriteOperand);
    }

    void Absolute(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHighExecute);

    }

    void AbsoluteRead(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHigh)
            .add(MicroOperation::ReadOperandAndExecute);

    }

    void AbsoluteWrite(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHigh)
            .add(MicroOperation::WriteOperand);

    }

    void AbsoluteRMW(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHigh)
            .add(MicroOperation::ReadOperand)
            .add(MicroOperation::DummyWrite)
            .add(MicroOperation::WriteOperand);

    }

    void ZeroPageX(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddXToZeroPageAddress)
            .add(MicroOperation::ReadOperandAndExecute);
    }

    void ZeroPageXRead(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddXToZeroPageAddress)
            .add(MicroOperation::ReadOperandAndExecute);
    }

    void ZeroPageXWrite(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddXToZeroPageAddress)
            .add(MicroOperation::WriteOperand);
    }

    void ZeroPageXRMW(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddXToZeroPageAddress)
            .add(MicroOperation::ReadOperand)
            .add(MicroOperation::DummyWrite)
            .add(MicroOperation::WriteOperand);
    }

    void ZeroPageY(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddYToZeroPageAddress)
            .add(MicroOperation::ReadOperandAndExecute);
    }

    void ZeroPageYRead(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddYToZeroPageAddress)
            .add(MicroOperation::ReadOperandAndExecute);
    }

    void ZeroPageYWrite(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddYToZeroPageAddress)
            .add(MicroOperation::WriteOperand);
    }

    void ZeroPageYRMW(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddYToZeroPageAddress)
            .add(MicroOperation::ReadOperand)
            .add(MicroOperation::DummyWrite)
            .add(MicroOperation::WriteOperand);
    }

    void AbsoluteX(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHighAddX)
            .add(MicroOperation::ReadOperandVariable);
    }

    void AbsoluteXRead(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHighAddX)
            .add(MicroOperation::ReadOperandVariable);
    }

    void AbsoluteXWrite(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHighAddX)
            .add(MicroOperation::DummyRead)
            .add(MicroOperation::WriteOperand);
    }

    void AbsoluteXRMW(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHighAddX)
            .add(MicroOperation::DummyRead)
            .add(MicroOperation::ReadOperand)
            .add(MicroOperation::DummyWrite)
            .add(MicroOperation::WriteOperand);
    }

    void AbsoluteY(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHighAddY)
            .add(MicroOperation::ReadOperandVariable);
    }

    void AbsoluteYRead(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHighAddY)
            .add(MicroOperation::ReadOperandVariable);
    }

    void AbsoluteYWrite(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHighAddY)
            .add(MicroOperation::DummyRead)
            .add(MicroOperation::WriteOperand);
    }

    void AbsoluteYRMW(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHighAddY)
            .add(MicroOperation::DummyRead)
            .add(MicroOperation::ReadOperand)
            .add(MicroOperation::DummyWrite)
            .add(MicroOperation::WriteOperand);
    }

    void Implied(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchOpcodeDiscardAndExec);
    }

    void ImpliedRead(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchOpcodeDiscardAndExec);
    }

    void ImpliedWrite(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchOpcodeDiscardAndExec);
    }

    void ImpliedRMW(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchOpcodeDiscardAndExec);
    }

    void Accumulator(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchOpcodeDiscardAndExec);
    }

    void Indirect(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHigh)
            .add(MicroOperation::FetchIndirectLow)
            // Puede existir el bug de cambio de página
            .add(MicroOperation::FetchIndirectHighExecute);
    }

    void IndirectRead(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchAddressLow)
            .add(MicroOperation::FetchAddressHigh)
            .add(MicroOperation::FetchIndirectLow)
            // Puede existir el bug de cambio de página
            .add(MicroOperation::FetchIndirectHighExecute);
    }

    void IndirectIndexedX(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddXToZeroPageAddress)
            .add(MicroOperation::FetchIndirectLow)
            //Puede existir el bug de cambio de página
            .add(MicroOperation::FetchIndirectHigh)
            .add(MicroOperation::ReadOperandAndExecute);
    }

    void IndirectIndexedXRead(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddXToZeroPageAddress)
            .add(MicroOperation::FetchIndirectLow)
            //Puede existir el bug de cambio de página
            .add(MicroOperation::FetchIndirectHigh)
            .add(MicroOperation::ReadOperandAndExecute);
    }

    void IndirectIndexedXWrite(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddXToZeroPageAddress)
            .add(MicroOperation::FetchIndirectLow)
            //Puede existir el bug de cambio de página
            .add(MicroOperation::FetchIndirectHigh)
            .add(MicroOperation::WriteOperand);
    }

    void IndirectIndexedXRMW(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::AddXToZeroPageAddress)
            .add(MicroOperation::FetchIndirectLow)
            //Puede existir el bug de cambio de página
            .add(MicroOperation::FetchIndirectHigh)
            .add(MicroOperation::ReadOperand)
            .add(MicroOperation::DummyWrite)
            .add(MicroOperation::WriteOperand);
    }

    void IndirectIndexedY(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::FetchIndirectLow)
            .add(MicroOperation::FetchIndirectHighAddY)
            .add(MicroOperation::ReadOperandVariable);
    }

    void IndirectIndexedYRead(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::FetchIndirectLow)
            .add(MicroOperation::FetchIndirectHighAddY)
            .add(MicroOperation::ReadOperandVariable);
    }

    void IndirectIndexedYWrite(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::FetchIndirectLow)
            .add(MicroOperation::FetchIndirectHighAddY)
            .add(MicroOperation::DummyRead)
            .add(MicroOperation::WriteOperand);
    }

    void IndirectIndexedYRMW(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::FetchIndirectLow)
            .add(MicroOperation::FetchIndirectHighAddY)
            .add(MicroOperation::DummyRead)
            .add(MicroOperation::ReadOperand)
            .add(MicroOperation::DummyWrite)
            .add(MicroOperation::WriteOperand);
    }

    void Relative(InstructionBuilder& builder)
    {
        builder
            .add(MicroOperation::FetchBranch);
    }
}