#pragma once

#include <cstdint>
#include <array>
#include <string_view>

namespace NES::CPU
{
    enum class MicroOperation : uint8_t
    {
        // Fetch
        FetchOpcode,
        FetchOperand,

        // Address
        FetchAddressLow,
        FetchAddressHigh,

        FetchZeroPageAddress,

        FetchIndirectLow,
        FetchIndirectHigh,

        // Address calculation
        AddXToAddress,
        AddYToAddress,

        AddXToZeroPageAddress,
        AddYToZeroPageAddress,

        // Memory
        ReadOperand,
        WriteOperand,

        // Bus cycles
        DummyRead,

        // Instruction
        Execute,
        EvaluateBranch
    };

    struct MicroOperationMapper 
    {
        // Arreglo indexado por el valor numérico del enum
        static constexpr std::array<std::string_view, 16> Strings = {
                // Fetch
            "FetchOpcode",
            "FetchOperand",

            // Address
            "FetchAddressLow",
            "FetchAddressHigh",

            "FetchZeroPageAddress",

            "FetchIndirectLow",
            "FetchIndirectHigh",

            // Address calculation
            "AddXToAddress",
            "AddYToAddress",

            "AddXToZeroPageAddress",
            "AddYToZeroPageAddress",

            // Memory
            "ReadOperand",
            "WriteOperand",

            // Bus cycles
            "DummyRead",

            // Instruction
            "Execute",
            "EvaluateBranch"
        };

        // Función estática para obtener la cadena original de forma segura
        static constexpr std::string_view ToString(MicroOperation mode) 
        {
            auto index = static_cast<size_t>(mode);
            if (index < Strings.size()) {
                return Strings[index];
            }
            return "Unknown";
        }
    };
}