#pragma once
#include <cstdint>

namespace NES::CPU
{
    class StatusRegister
    {
        public:
            enum FlagID: uint8_t
            {
                C = 0,
                Z = 1,
                I = 2,
                D = 3,
                B = 4,
                U = 5,
                V = 6,
                N = 7
            };

            StatusRegister();


            void setFlag(FlagID flag, bool value);

            bool getFlag(FlagID flag) const;

            uint8_t getByte() const;

            void setByte(uint8_t value);

            void applyMask(uint8_t values, uint8_t mask);
        private:

            uint8_t flags;
    };
}