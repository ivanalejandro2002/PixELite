#include <NES/CPU/Instruction/InstructionTable.h>

namespace NES::CPU
{
    InstructionTable::InstructionTable()
    {
        table.fill({
            InstructionOperation::Undefined,
            AddressingMode::Implied,
            AccessType::None,
            0
        });
        
        buildTable();
    }

    void InstructionTable::set(
        uint8_t opcode,
        InstructionOperation operation,
        AddressingMode addressingMode,
        AccessType accessType,
        uint8_t cycles
    )
    {
        table[opcode] = {
            operation,
            addressingMode,
            accessType,
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
        set(0x00, InstructionOperation::BRK, AddressingMode::Implied,           AccessType::None,               7);
        set(0x01, InstructionOperation::ORA, AddressingMode::IndirectIndexedX,  AccessType::Read,               6);
        set(0x02, InstructionOperation::KIL, AddressingMode::Implied,           AccessType::None,               2);
        set(0x03, InstructionOperation::SLO, AddressingMode::IndirectIndexedX,  AccessType::ReadModifyWrite,    8);
        set(0x04, InstructionOperation::NOP, AddressingMode::ZeroPage,          AccessType::None,               3);
        set(0x05, InstructionOperation::ORA, AddressingMode::ZeroPage,          AccessType::Read,               3);
        set(0x06, InstructionOperation::ASL, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0x07, InstructionOperation::SLO, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0x08, InstructionOperation::PHP, AddressingMode::Implied,           AccessType::Write,              3);
        set(0x09, InstructionOperation::ORA, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0x0A, InstructionOperation::ASL, AddressingMode::Accumulator,       AccessType::None,               2);
        set(0x0B, InstructionOperation::ANC, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0x0C, InstructionOperation::NOP, AddressingMode::Absolute,          AccessType::None,               3);
        set(0x0D, InstructionOperation::ORA, AddressingMode::Absolute,          AccessType::Read,               4);
        set(0x0E, InstructionOperation::ASL, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        set(0x0F, InstructionOperation::SLO, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        //0x10 - 0x1F
        set(0x10, InstructionOperation::BPL, AddressingMode::Relative,          AccessType::None,               2);
        set(0x11, InstructionOperation::ORA, AddressingMode::IndirectIndexedY,  AccessType::Read,               5);
        set(0x12, InstructionOperation::KIL, AddressingMode::Implied,           AccessType::None,               2);
        set(0x13, InstructionOperation::SLO, AddressingMode::IndirectIndexedY,  AccessType::ReadModifyWrite,    8);
        set(0x14, InstructionOperation::NOP, AddressingMode::ZeroPageX,         AccessType::None,               4);
        set(0x15, InstructionOperation::ORA, AddressingMode::ZeroPageX,         AccessType::Read,               4);
        set(0x16, InstructionOperation::ASL, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0x17, InstructionOperation::SLO, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0x18, InstructionOperation::CLC, AddressingMode::Implied,           AccessType::None,               2);
        set(0x19, InstructionOperation::ORA, AddressingMode::AbsoluteY,         AccessType::Read,               4);
        set(0x1A, InstructionOperation::NOP, AddressingMode::Implied,           AccessType::Read,               2);
        set(0x1B, InstructionOperation::SLO, AddressingMode::AbsoluteY,         AccessType::ReadModifyWrite,    7);
        set(0x1C, InstructionOperation::NOP, AddressingMode::AbsoluteX,         AccessType::None,               4);
        set(0x1D, InstructionOperation::ORA, AddressingMode::AbsoluteX,         AccessType::Read,               4);
        set(0x1E, InstructionOperation::ASL, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
        set(0x1F, InstructionOperation::SLO, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
        //0x20 - 0x2F
        set(0x20, InstructionOperation::JSR, AddressingMode::Absolute,          AccessType::None,               6);
        set(0x21, InstructionOperation::AND, AddressingMode::IndirectIndexedX,  AccessType::Read,               6);
        set(0x22, InstructionOperation::KIL, AddressingMode::Implied,           AccessType::None,               2);
        set(0x23, InstructionOperation::RLA, AddressingMode::IndirectIndexedX,  AccessType::ReadModifyWrite,    8);
        set(0x24, InstructionOperation::BIT, AddressingMode::ZeroPage,          AccessType::Read,               3);
        set(0x25, InstructionOperation::AND, AddressingMode::ZeroPage,          AccessType::Read,               3);
        set(0x26, InstructionOperation::ROL, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0x27, InstructionOperation::RLA, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0x28, InstructionOperation::PLP, AddressingMode::Implied,           AccessType::Write,              4);
        set(0x29, InstructionOperation::AND, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0x2A, InstructionOperation::ROL, AddressingMode::Accumulator,       AccessType::None,               2);
        set(0x2B, InstructionOperation::ANC, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0x2C, InstructionOperation::BIT, AddressingMode::Absolute,          AccessType::Read,               4);
        set(0x2D, InstructionOperation::AND, AddressingMode::Absolute,          AccessType::Read,               4);
        set(0x2E, InstructionOperation::ROL, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        set(0x2F, InstructionOperation::RLA, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        //0x30 - 0x3F
        set(0x30, InstructionOperation::BMI, AddressingMode::Relative,          AccessType::None,               2);
        set(0x31, InstructionOperation::AND, AddressingMode::IndirectIndexedY,  AccessType::Read,               5);
        set(0x32, InstructionOperation::KIL, AddressingMode::Implied,           AccessType::None,               2);
        set(0x33, InstructionOperation::RLA, AddressingMode::IndirectIndexedY,  AccessType::ReadModifyWrite,    8);
        set(0x34, InstructionOperation::NOP, AddressingMode::ZeroPageX,         AccessType::None,               4);
        set(0x35, InstructionOperation::AND, AddressingMode::ZeroPageX,         AccessType::Read,               4);
        set(0x36, InstructionOperation::ROL, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0x37, InstructionOperation::RLA, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0x38, InstructionOperation::SEC, AddressingMode::Implied,           AccessType::None,               2);
        set(0x39, InstructionOperation::AND, AddressingMode::AbsoluteY,         AccessType::Read,               4);
        set(0x3A, InstructionOperation::NOP, AddressingMode::Implied,           AccessType::Read,               2);
        set(0x3B, InstructionOperation::RLA, AddressingMode::AbsoluteY,         AccessType::ReadModifyWrite,    7);
        set(0x3C, InstructionOperation::NOP, AddressingMode::AbsoluteX,         AccessType::None,               4);
        set(0x3D, InstructionOperation::AND, AddressingMode::AbsoluteX,         AccessType::Read,               4);
        set(0x3E, InstructionOperation::ROL, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
        set(0x3F, InstructionOperation::RLA, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
        //0x40 - 0x4F
        set(0x40, InstructionOperation::RTI, AddressingMode::Implied,           AccessType::None,               6);
        set(0x41, InstructionOperation::EOR, AddressingMode::IndirectIndexedX,  AccessType::Read,               6);
        set(0x42, InstructionOperation::KIL, AddressingMode::Implied,           AccessType::None,               2);
        set(0x43, InstructionOperation::SRE, AddressingMode::IndirectIndexedX,  AccessType::ReadModifyWrite,    8);
        set(0x44, InstructionOperation::NOP, AddressingMode::ZeroPage,          AccessType::None,               3);
        set(0x45, InstructionOperation::EOR, AddressingMode::ZeroPage,          AccessType::Read,               3);
        set(0x46, InstructionOperation::LSR, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0x47, InstructionOperation::SRE, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0x48, InstructionOperation::PHA, AddressingMode::Implied,           AccessType::Write,              3);
        set(0x49, InstructionOperation::EOR, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0x4A, InstructionOperation::LSR, AddressingMode::Accumulator,       AccessType::None,               2);
        set(0x4B, InstructionOperation::ALR, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0x4C, InstructionOperation::JMP, AddressingMode::Absolute,          AccessType::None,               3);
        set(0x4D, InstructionOperation::EOR, AddressingMode::Absolute,          AccessType::Read,               4);
        set(0x4E, InstructionOperation::LSR, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        set(0x4F, InstructionOperation::SRE, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        //0x50 - 0x5F
        set(0x50, InstructionOperation::BVC, AddressingMode::Relative,          AccessType::None,               2);
        set(0x51, InstructionOperation::EOR, AddressingMode::IndirectIndexedY,  AccessType::Read,               5);
        set(0x52, InstructionOperation::KIL, AddressingMode::Implied,           AccessType::None,               2);
        set(0x53, InstructionOperation::SRE, AddressingMode::IndirectIndexedY,  AccessType::ReadModifyWrite,    8);
        set(0x54, InstructionOperation::NOP, AddressingMode::ZeroPageX,         AccessType::None,               4);
        set(0x55, InstructionOperation::EOR, AddressingMode::ZeroPageX,         AccessType::Read,               4);
        set(0x56, InstructionOperation::LSR, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0x57, InstructionOperation::SRE, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0x58, InstructionOperation::CLI, AddressingMode::Implied,           AccessType::None,               2);
        set(0x59, InstructionOperation::EOR, AddressingMode::AbsoluteY,         AccessType::Read,               4);
        set(0x5A, InstructionOperation::NOP, AddressingMode::Implied,           AccessType::Read,               2);
        set(0x5B, InstructionOperation::SRE, AddressingMode::AbsoluteY,         AccessType::ReadModifyWrite,    7);
        set(0x5C, InstructionOperation::NOP, AddressingMode::AbsoluteX,         AccessType::None,               4);
        set(0x5D, InstructionOperation::EOR, AddressingMode::AbsoluteX,         AccessType::Read,               4);
        set(0x5E, InstructionOperation::LSR, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
        set(0x5F, InstructionOperation::SRE, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
        //0x60 - 0x6F
        set(0x60, InstructionOperation::RTS, AddressingMode::Implied,           AccessType::None,               6);
        set(0x61, InstructionOperation::ADC, AddressingMode::IndirectIndexedX,  AccessType::Read,               6);
        set(0x62, InstructionOperation::KIL, AddressingMode::Implied,           AccessType::None,               2);
        set(0x63, InstructionOperation::RRA, AddressingMode::IndirectIndexedX,  AccessType::ReadModifyWrite,    8);
        set(0x64, InstructionOperation::NOP, AddressingMode::ZeroPage,          AccessType::None,               3);
        set(0x65, InstructionOperation::ADC, AddressingMode::ZeroPage,          AccessType::Read,               3);
        set(0x66, InstructionOperation::ROR, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0x67, InstructionOperation::RRA, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0x68, InstructionOperation::PLA, AddressingMode::Implied,           AccessType::Write,              4);
        set(0x69, InstructionOperation::ADC, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0x6A, InstructionOperation::ROR, AddressingMode::Accumulator,       AccessType::ReadModifyWrite,    2);
        set(0x6B, InstructionOperation::ARR, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0x6C, InstructionOperation::JMP, AddressingMode::Indirect,          AccessType::None,               5);
        set(0x6D, InstructionOperation::ADC, AddressingMode::Absolute,          AccessType::Read,               4);
        set(0x6E, InstructionOperation::ROR, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        set(0x6F, InstructionOperation::RRA, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        //0x70 - 0x7F
        set(0x70, InstructionOperation::BVS, AddressingMode::Relative,          AccessType::None,               2);
        set(0x71, InstructionOperation::ADC, AddressingMode::IndirectIndexedY,  AccessType::Read,               5);
        set(0x72, InstructionOperation::KIL, AddressingMode::Implied,           AccessType::None,               2);
        set(0x73, InstructionOperation::RRA, AddressingMode::IndirectIndexedY,  AccessType::ReadModifyWrite,    8);
        set(0x74, InstructionOperation::NOP, AddressingMode::ZeroPageX,         AccessType::None,               4);
        set(0x75, InstructionOperation::ADC, AddressingMode::ZeroPageX,         AccessType::Read,               4);
        set(0x76, InstructionOperation::ROR, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0x77, InstructionOperation::RRA, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0x78, InstructionOperation::SEI, AddressingMode::Implied,           AccessType::None,               2);
        set(0x79, InstructionOperation::ADC, AddressingMode::AbsoluteY,         AccessType::Read,               4);
        set(0x7A, InstructionOperation::NOP, AddressingMode::Implied,           AccessType::Read,               2);
        set(0x7B, InstructionOperation::RRA, AddressingMode::AbsoluteY,         AccessType::ReadModifyWrite,    7);
        set(0x7C, InstructionOperation::NOP, AddressingMode::AbsoluteX,         AccessType::None,               4);
        set(0x7D, InstructionOperation::ADC, AddressingMode::AbsoluteX,         AccessType::Read,               4);
        set(0x7E, InstructionOperation::ROR, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
        set(0x7F, InstructionOperation::RRA, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
        //0x80 - 0x8F
        set(0x80, InstructionOperation::NOP, AddressingMode::Immediate,         AccessType::None,               2);
        set(0x81, InstructionOperation::STA, AddressingMode::IndirectIndexedX,  AccessType::Write,              6);
        set(0x82, InstructionOperation::NOP, AddressingMode::Immediate,         AccessType::None,               2);
        set(0x83, InstructionOperation::SAX, AddressingMode::IndirectIndexedX,  AccessType::ReadModifyWrite,    8);
        set(0x84, InstructionOperation::STY, AddressingMode::ZeroPage,          AccessType::Write,              3);
        set(0x85, InstructionOperation::STA, AddressingMode::ZeroPage,          AccessType::Write,              3);
        set(0x86, InstructionOperation::STX, AddressingMode::ZeroPage,          AccessType::Write,              3);
        set(0x87, InstructionOperation::SAX, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0x88, InstructionOperation::DEY, AddressingMode::Implied,           AccessType::None,               2);
        set(0x89, InstructionOperation::NOP, AddressingMode::Immediate,         AccessType::None,               2);
        set(0x8A, InstructionOperation::TXA, AddressingMode::Implied,           AccessType::None,               2);
        set(0x8B, InstructionOperation::XAA, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0x8C, InstructionOperation::STY, AddressingMode::Absolute,          AccessType::Write,              4);
        set(0x8D, InstructionOperation::STA, AddressingMode::Absolute,          AccessType::Write,              4);
        set(0x8E, InstructionOperation::STX, AddressingMode::Absolute,          AccessType::Write,              4);
        set(0x8F, InstructionOperation::SAX, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        //0x90 - 0x9F
        set(0x90, InstructionOperation::BCC, AddressingMode::Relative,          AccessType::None,               2);
        set(0x91, InstructionOperation::STA, AddressingMode::IndirectIndexedY,  AccessType::Write,              6);
        set(0x92, InstructionOperation::KIL, AddressingMode::Implied,           AccessType::None,               2);
        set(0x93, InstructionOperation::AHX, AddressingMode::IndirectIndexedY,  AccessType::Write,              6);
        set(0x94, InstructionOperation::STY, AddressingMode::ZeroPageX,         AccessType::Write,              4);
        set(0x95, InstructionOperation::STA, AddressingMode::ZeroPageX,         AccessType::Write,              4);
        set(0x96, InstructionOperation::STX, AddressingMode::ZeroPageY,         AccessType::Write,              4);
        set(0x97, InstructionOperation::SAX, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0x98, InstructionOperation::TYA, AddressingMode::Implied,           AccessType::None,               2);
        set(0x99, InstructionOperation::STA, AddressingMode::AbsoluteY,         AccessType::Write,              5);
        set(0x9A, InstructionOperation::TXS, AddressingMode::Implied,           AccessType::None,               2);
        set(0x9B, InstructionOperation::TAS, AddressingMode::AbsoluteY,         AccessType::Write,              5);
        set(0x9C, InstructionOperation::SHY, AddressingMode::AbsoluteX,         AccessType::Write,              5);
        set(0x9D, InstructionOperation::STA, AddressingMode::AbsoluteX,         AccessType::Write,              5);
        set(0x9E, InstructionOperation::SHX, AddressingMode::AbsoluteY,         AccessType::Write,              5);
        set(0x9F, InstructionOperation::AHX, AddressingMode::AbsoluteY,         AccessType::Write,              5);
        //0xA0 - 0xAF
        set(0xA0, InstructionOperation::LDY, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0xA1, InstructionOperation::LDA, AddressingMode::IndirectIndexedX,  AccessType::Read,               6);
        set(0xA2, InstructionOperation::LDX, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0xA3, InstructionOperation::LAX, AddressingMode::IndirectIndexedX,  AccessType::ReadModifyWrite,    8);
        set(0xA4, InstructionOperation::LDY, AddressingMode::ZeroPage,          AccessType::Read,               3);
        set(0xA5, InstructionOperation::LDA, AddressingMode::ZeroPage,          AccessType::Read,               3);
        set(0xA6, InstructionOperation::LDX, AddressingMode::ZeroPage,          AccessType::Read,               3);
        set(0xA7, InstructionOperation::LAX, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0xA8, InstructionOperation::TAY, AddressingMode::Implied,           AccessType::None,               2);
        set(0xA9, InstructionOperation::LDA, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0xAA, InstructionOperation::TAX, AddressingMode::Implied,           AccessType::None,               2);
        set(0xAB, InstructionOperation::LAX, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0xAC, InstructionOperation::LDY, AddressingMode::Absolute,          AccessType::Read,               4);
        set(0xAD, InstructionOperation::LDA, AddressingMode::Absolute,          AccessType::Read,               4);
        set(0xAE, InstructionOperation::LDX, AddressingMode::Absolute,          AccessType::Read,               4);
        set(0xAF, InstructionOperation::LAX, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        //0xB0 - 0xBF
        set(0xB0, InstructionOperation::BCS, AddressingMode::Relative,          AccessType::None,               2);
        set(0xB1, InstructionOperation::LDA, AddressingMode::IndirectIndexedY,  AccessType::Read,               5);
        set(0xB2, InstructionOperation::KIL, AddressingMode::Implied,           AccessType::None,               2);
        set(0xB3, InstructionOperation::LAX, AddressingMode::IndirectIndexedY,  AccessType::ReadModifyWrite,    8);
        set(0xB4, InstructionOperation::LDY, AddressingMode::ZeroPageX,         AccessType::Read,               4);
        set(0xB5, InstructionOperation::LDA, AddressingMode::ZeroPageX,         AccessType::Read,               4);
        set(0xB6, InstructionOperation::LDX, AddressingMode::ZeroPageY,         AccessType::Read,               4);
        set(0xB7, InstructionOperation::LAX, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0xB8, InstructionOperation::CLV, AddressingMode::Implied,           AccessType::None,               2);
        set(0xB9, InstructionOperation::LDA, AddressingMode::AbsoluteY,         AccessType::Read,               4);
        set(0xBA, InstructionOperation::TSX, AddressingMode::Implied,           AccessType::None,               2);
        set(0xBB, InstructionOperation::LAS, AddressingMode::AbsoluteY,         AccessType::Read,               4);
        set(0xBC, InstructionOperation::LDY, AddressingMode::AbsoluteX,         AccessType::Read,               4);
        set(0xBD, InstructionOperation::LDA, AddressingMode::AbsoluteX,         AccessType::Read,               4);
        set(0xBE, InstructionOperation::LDX, AddressingMode::AbsoluteY,         AccessType::Read,               4);
        set(0xBF, InstructionOperation::LAX, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
        //0xC0 - 0xCF
        set(0xC0, InstructionOperation::CPY, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0xC1, InstructionOperation::CMP, AddressingMode::IndirectIndexedX,  AccessType::Read,               6);
        set(0xC2, InstructionOperation::NOP, AddressingMode::Immediate,         AccessType::None,               2);
        set(0xC3, InstructionOperation::DCP, AddressingMode::IndirectIndexedX,  AccessType::ReadModifyWrite,    8);
        set(0xC4, InstructionOperation::CPY, AddressingMode::ZeroPage,          AccessType::Read,               3);
        set(0xC5, InstructionOperation::CMP, AddressingMode::ZeroPage,          AccessType::Read,               3);
        set(0xC6, InstructionOperation::DEC, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0xC7, InstructionOperation::DCP, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0xC8, InstructionOperation::INY, AddressingMode::Implied,           AccessType::None,               2);
        set(0xC9, InstructionOperation::CMP, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0xCA, InstructionOperation::DEX, AddressingMode::Implied,           AccessType::None,               2);
        set(0xCB, InstructionOperation::AXS, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0xCC, InstructionOperation::CPY, AddressingMode::Absolute,          AccessType::Read,               4);
        set(0xCD, InstructionOperation::CMP, AddressingMode::Absolute,          AccessType::Read,               4);
        set(0xCE, InstructionOperation::DEC, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        set(0xCF, InstructionOperation::DCP, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        //0xD0 - 0xDF
        set(0xD0, InstructionOperation::BNE, AddressingMode::Relative,          AccessType::None,               2);
        set(0xD1, InstructionOperation::CMP, AddressingMode::IndirectIndexedY,  AccessType::Read,               5);
        set(0xD2, InstructionOperation::KIL, AddressingMode::Implied,           AccessType::None,               2);
        set(0xD3, InstructionOperation::DCP, AddressingMode::IndirectIndexedY,  AccessType::ReadModifyWrite,    8);
        set(0xD4, InstructionOperation::NOP, AddressingMode::ZeroPageX,         AccessType::None,               4);
        set(0xD5, InstructionOperation::CMP, AddressingMode::ZeroPageX,         AccessType::Read,               4);
        set(0xD6, InstructionOperation::DEC, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0xD7, InstructionOperation::DCP, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0xD8, InstructionOperation::CLD, AddressingMode::Implied,           AccessType::None,               2);
        set(0xD9, InstructionOperation::CMP, AddressingMode::AbsoluteY,         AccessType::Read,               4);
        set(0xDA, InstructionOperation::NOP, AddressingMode::Implied,           AccessType::Read,               2);
        set(0xDB, InstructionOperation::DCP, AddressingMode::AbsoluteY,         AccessType::ReadModifyWrite,    7);
        set(0xDC, InstructionOperation::NOP, AddressingMode::AbsoluteX,         AccessType::None,               4);
        set(0xDD, InstructionOperation::CMP, AddressingMode::AbsoluteX,         AccessType::Read,               4);
        set(0xDE, InstructionOperation::DEC, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
        set(0xDF, InstructionOperation::DCP, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
        //0xE0 - 0xEF
        set(0xE0, InstructionOperation::CPX, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0xE1, InstructionOperation::SBC, AddressingMode::IndirectIndexedX,  AccessType::Read,               6);
        set(0xE2, InstructionOperation::NOP, AddressingMode::Immediate,         AccessType::None,               2);
        set(0xE3, InstructionOperation::ISC, AddressingMode::IndirectIndexedX,  AccessType::ReadModifyWrite,    8);
        set(0xE4, InstructionOperation::CPX, AddressingMode::ZeroPage,          AccessType::Read,               3);
        set(0xE5, InstructionOperation::SBC, AddressingMode::ZeroPage,          AccessType::Read,               3);
        set(0xE6, InstructionOperation::INC, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0xE7, InstructionOperation::ISC, AddressingMode::ZeroPage,          AccessType::ReadModifyWrite,    5);
        set(0xE8, InstructionOperation::INX, AddressingMode::Implied,           AccessType::None,               2);
        set(0xE9, InstructionOperation::SBC, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0xEA, InstructionOperation::NOP, AddressingMode::Implied,           AccessType::Read,               2);
        set(0xEB, InstructionOperation::SBC, AddressingMode::Immediate,         AccessType::Read,               2);
        set(0xEC, InstructionOperation::CPX, AddressingMode::Absolute,          AccessType::Read,               4);
        set(0xED, InstructionOperation::SBC, AddressingMode::Absolute,          AccessType::Read,               4);
        set(0xEE, InstructionOperation::INC, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        set(0xEF, InstructionOperation::ISC, AddressingMode::Absolute,          AccessType::ReadModifyWrite,    6);
        //0xF0 - 0xFF
        set(0xF0, InstructionOperation::BEQ, AddressingMode::Relative,          AccessType::None,               2);
        set(0xF1, InstructionOperation::SBC, AddressingMode::IndirectIndexedY,  AccessType::Read,               5);
        set(0xF2, InstructionOperation::KIL, AddressingMode::Implied,           AccessType::None,               2);
        set(0xF3, InstructionOperation::ISC, AddressingMode::IndirectIndexedY,  AccessType::ReadModifyWrite,    8);
        set(0xF4, InstructionOperation::NOP, AddressingMode::ZeroPageX,         AccessType::None,               4);
        set(0xF5, InstructionOperation::SBC, AddressingMode::ZeroPageX,         AccessType::Read,               4);
        set(0xF6, InstructionOperation::INC, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0xF7, InstructionOperation::ISC, AddressingMode::ZeroPageX,         AccessType::ReadModifyWrite,    6);
        set(0xF8, InstructionOperation::SED, AddressingMode::Implied,           AccessType::None,               2);
        set(0xF9, InstructionOperation::SBC, AddressingMode::AbsoluteY,         AccessType::Read,               4);
        set(0xFA, InstructionOperation::NOP, AddressingMode::Implied,           AccessType::Read,               2);
        set(0xFB, InstructionOperation::ISC, AddressingMode::AbsoluteY,         AccessType::ReadModifyWrite,    7);
        set(0xFC, InstructionOperation::NOP, AddressingMode::AbsoluteX,         AccessType::None,               4);
        set(0xFD, InstructionOperation::SBC, AddressingMode::AbsoluteX,         AccessType::Read,               4);
        set(0xFE, InstructionOperation::INC, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
        set(0xFF, InstructionOperation::ISC, AddressingMode::AbsoluteX,         AccessType::ReadModifyWrite,    7);
    }
}