#pragma once

#include <cstdint>
#include <array>
#include <string_view>

namespace NES::CPU
{
    enum class AddressingMode: uint8_t
    {
        Implied,
        Accumulator,
        Immediate,
        ZeroPage,
        ZeroPageX,
        ZeroPageY,
        Absolute,
        AbsoluteX,
        AbsoluteY,
        Indirect,
        IndirectIndexedX,
        IndirectIndexedY,
        Relative
    };


    struct AddressingModeMapper 
    {
        // Arreglo indexado por el valor numérico del enum
        static constexpr std::array<std::string_view, 13> Strings = {
            "Implied",
            "Accumulator",
            "Immediate",
            "ZeroPage",
            "ZeroPageX",
            "ZeroPageY",
            "Absolute",
            "AbsoluteX",
            "AbsoluteY",
            "Indirect",
            "IndirectIndexedX",
            "IndirectIndexedY",
            "Relative"
        };

        // Función estática para obtener la cadena original de forma segura
        static constexpr std::string_view ToString(AddressingMode mode) 
        {
            auto index = static_cast<size_t>(mode);
            if (index < Strings.size()) {
                return Strings[index];
            }
            return "Unknown";
        }
    };
}