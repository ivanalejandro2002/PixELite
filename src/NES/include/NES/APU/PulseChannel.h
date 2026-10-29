#pragma once
#include <cstdint>

namespace NES
{
    class PulseChannel
    {
        public:
            PulseChannel(bool isPulse1);
            ~PulseChannel() = default;

            void writeControl(uint8_t value);
            void writeSweep(uint8_t value);
            void writeTimerLow(uint8_t value);
            void writeTimerHigh(uint8_t value);

            void clockTimer();
            void clockEnvelope();
            void clockLengthCounter();
            void clockSweep();

            uint8_t output() const;
            uint8_t getLengthCounter() const;
            void setEnabled(bool enable);

        private:
            bool isPulse1;
            bool enabled;

            uint8_t dutyCycle;
            bool lengthCounterEnable;
            bool constantVolume;
            uint8_t volumeOrEnvelopeReload;

            uint16_t timerReload;
            uint16_t timerCounter;
            uint8_t sequencerStep;

            uint8_t lengthCounter;

            bool envelopeStartFlag;
            uint8_t envelopeVolume;
            uint8_t envelopeDivider;

            bool sweepEnabled;
            bool sweepReloadFlag;
            bool sweepNegate;
            uint8_t sweepShift;
            uint8_t sweepDivider;

            static const uint8_t LENGTH_TABLE[32];
    };
}