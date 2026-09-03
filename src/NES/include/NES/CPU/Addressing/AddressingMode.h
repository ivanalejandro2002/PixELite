#pragma once

#include <cstdint>

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
}