#include <NES/CPU/Instruction/InstructionTable.h>

namespace NES::CPU
{
    InstructionTable::InstructionTable()
    {
        table.fill({
            InstructionOperation::Undefined,
            AddressingMode::Implied,
            0
        });
        
        buildTable();
    }

    void InstructionTable::set(
        uint8_t opcode,
        InstructionOperation operation,
        AddressingMode addressingMode,
        uint8_t cycles
    )
    {
        table[opcode] = {
            operation,
            addressingMode,
            cycles
        };
    }

    const InstructionDefinition& InstructionTable::get(uint8_t opcode)const
    {
        return table[opcode];
    }

    void InstructionTable::buildTable()
    {
        //0x00 - 0x0F
        //0x10 - 0x1F
        //0x20 - 0x2F
        //0x30 - 0x3F
        //0x40 - 0x4F
        //0x50 - 0x5F
        //0x60 - 0x6F
        set(0x61, InstructionOperation::ADC, AddressingMode::IndirectIndexedX, 6);
        set(0x65, InstructionOperation::ADC, AddressingMode::ZeroPage, 3);
        set(0x69, InstructionOperation::ADC, AddressingMode::Immediate, 2);
        set(0x6D, InstructionOperation::ADC, AddressingMode::Absolute, 4);
        //0x70 - 0x7F
        set(0x71, InstructionOperation::ADC, AddressingMode::IndirectIndexedY, 5);
        set(0x75, InstructionOperation::ADC, AddressingMode::ZeroPageX, 4);
        set(0x79, InstructionOperation::ADC, AddressingMode::AbsoluteY, 4);
        set(0x7D, InstructionOperation::ADC, AddressingMode::AbsoluteX, 4);
        //0x80 - 0x8F
        //0x90 - 0x9F
        //0xA0 - 0xAF
        set(0xA1, InstructionOperation::LDA, AddressingMode::IndirectIndexedX, 6);
        set(0xA5, InstructionOperation::LDA, AddressingMode::ZeroPage, 3);
        set(0xA9, InstructionOperation::LDA, AddressingMode::Immediate, 2);
        set(0xAD, InstructionOperation::LDA, AddressingMode::Absolute, 4);
        //0xB0 - 0xBF
        set(0xB1, InstructionOperation::LDA, AddressingMode::IndirectIndexedY, 5);
        set(0xB5, InstructionOperation::LDA, AddressingMode::ZeroPageX, 4);
        set(0xB9, InstructionOperation::LDA, AddressingMode::AbsoluteY, 2);
        set(0xBD, InstructionOperation::LDA, AddressingMode::AbsoluteX, 4);
        //0xC0 - 0xCF
        //0xD0 - 0xDF
        //0xE0 - 0xEF
        //0xF0 - 0xFF
    }
}