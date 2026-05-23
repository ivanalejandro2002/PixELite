#pragma once

#include <NES/CPU/StatusRegister.h>

namespace NES::CPU
{
    class ALUResult
    {
        public:
            enum FlagMask : uint8_t
            {
                NONE = 0,

                C = 1 << static_cast<uint8_t>(
                    StatusRegister::FlagID::C
                ),

                Z = 1 << static_cast<uint8_t>(
                    StatusRegister::FlagID::Z
                ),

                I = 1 << static_cast<uint8_t>(
                    StatusRegister::FlagID::I
                ),

                D = 1 << static_cast<uint8_t>(
                    StatusRegister::FlagID::D
                ),

                B = 1 << static_cast<uint8_t>(
                    StatusRegister::FlagID::B
                ),

                U = 1 << static_cast<uint8_t>(
                    StatusRegister::FlagID::U
                ),

                V = 1 << static_cast<uint8_t>(
                    StatusRegister::FlagID::V
                ),

                N = 1 << static_cast<uint8_t>(
                    StatusRegister::FlagID::N
                )
            };

            uint8_t result;
            uint8_t statusBits;
            uint8_t statusChangedBits;

            ALUResult(uint8_t value, uint8_t mode, uint8_t changedbits);
    };
}