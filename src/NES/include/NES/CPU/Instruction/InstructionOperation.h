#pragma once

#include <cstdint>
#include <array>
#include <string_view>

namespace NES::CPU
{
    enum class InstructionOperation : uint8_t
    {
        LDA,
        LDX,
        LDY,

        STA,
        STX,
        STY,

        TAX,
        TAY,
        TXA,
        TYA,

        ADC,
        SBC,
        INC,
        DEC,
        INX,
        DEX,
        INY,
        DEY,

        ASL,
        LSR,
        ROL,
        ROR,

        AND,
        ORA,
        EOR,
        BIT,

        CMP,
        CPX,
        CPY,

        BCC,
        BCS,
        BEQ,
        BNE,
        BPL,
        BMI,
        BVC,
        BVS,

        JMP,
        JSR,
        RTS,
        BRK,
        RTI,

        PHA,
        PLA,
        PHP,
        PLP,
        TXS,
        TSX,

        CLC,
        SEC,
        CLI,
        SEI,
        CLD,
        SED,
        CLV,

        NOP,

        SLO,
        RLA,
        SRE,
        RRA,
        SAX,
        LAX,
        DCP,
        ISC,

        ANC,
        ALR,
        ARR,
        XAA,
        AXS,

        AHX,
        SHX,
        SHY,

        TAS,
        LAS,

        KIL,
        Undefined
    };


    struct InstructionOperationMapper 
    {
        // Arreglo indexado por el valor numérico del enum
        static constexpr std::array<std::string_view, 76> Strings = {
            "LDA",
            "LDX",
            "LDY",

            "STA",
            "STX",
            "STY",

            "TAX",
            "TAY",
            "TXA",
            "TYA",

            "ADC",
            "SBC",
            "INC",
            "DEC",
            "INX",
            "DEX",
            "INY",
            "DEY",

            "ASL",
            "LSR",
            "ROL",
            "ROR",

            "AND",
            "ORA",
            "EOR",
            "BIT",

            "CMP",
            "CPX",
            "CPY",

            "BCC",
            "BCS",
            "BEQ",
            "BNE",
            "BPL",
            "BMI",
            "BVC",
            "BVS",

            "JMP",
            "JSR",
            "RTS",
            "BRK",
            "RTI",

            "PHA",
            "PLA",
            "PHP",
            "PLP",
            "TXS",
            "TSX",

            "CLC",
            "SEC",
            "CLI",
            "SEI",
            "CLD",
            "SED",
            "CLV",

            "NOP",

            "SLO",
            "RLA",
            "SRE",
            "RRA",
            "SAX",
            "LAX",
            "DCP",
            "ISC",

            "ANC",
            "ALR",
            "ARR",
            "XAA",
            "AXS",

            "AHX",
            "SHX",
            "SHY",

            "TAS",
            "LAS",

            "KIL",
            "Undefined"
        };

        // Función estática para obtener la cadena original de forma segura
        static constexpr std::string_view ToString(InstructionOperation mode) 
        {
            auto index = static_cast<size_t>(mode);
            if (index < Strings.size()) {
                return Strings[index];
            }
            return "Unknown";
        }
    };


    enum class AccessType : uint8_t
    {
        None,
        Read,
        Write,
        ReadModifyWrite
    };

    struct AccessTypeMapper 
    {
        // Arreglo indexado por el valor numérico del enum
        static constexpr std::array<std::string_view, 4> Strings = {
            "None",
            "Read",
            "Write",
            "ReadModifyWrite"
        };

        // Función estática para obtener la cadena original de forma segura
        static constexpr std::string_view ToString(AccessType mode) 
        {
            auto index = static_cast<size_t>(mode);
            if (index < Strings.size()) {
                return Strings[index];
            }
            return "Unknown";
        }
    };

}