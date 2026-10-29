#pragma once
#include <cstdint>

namespace NES
{
    class TriangleChannel
    {
        public:
            TriangleChannel();
            ~TriangleChannel() = default;

            void writeControl(uint8_t value);
            void writeTimerLow(uint8_t value);
            void writeTimerHigh(uint8_t value);

            void clockTimer();
            void clockLinearCounter();
            void clockLengthCounter();

            uint8_t output() const;
            uint8_t getLengthCounter() const;
            void setEnabled(bool enable);

        private:
            bool enabled;

            bool controlFlag;
            uint8_t linearCounterReloadValue;

            uint16_t timerReload;
            uint16_t timerCounter;
            uint8_t sequencerStep;

            uint8_t linearCounter;
            bool linearcounterReloadFlag;
            uint8_t lengthCounter;

            static const uint8_t LENGTH_TABLE[32];
            static const uint8_t TRIANGLE_SEQUENCE[32];
    };
}