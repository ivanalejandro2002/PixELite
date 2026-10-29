#pragma once
#include <cstdint>

namespace NES
{
    class NoiseChannel
    {
        public:
            NoiseChannel();
            ~NoiseChannel() = default;

            void writeControl(uint8_t value);
            void writePeriod(uint8_t value);
            void writeLength(uint8_t value);

            void clockTimer();
            void clockEnvelope();
            void clockLengthCounter();

            uint8_t output() const;
            uint8_t getLengthCounter() const;
            void setEnabled(bool enable);

        private:
            bool enabled;

            bool lengthCounterEnable;
            bool constantVolume;
            uint8_t volumeOrEnvelopeReload;

            bool modeFlag;
            uint16_t timerReload;

            uint8_t lengthCounter;

            uint16_t timerCounter;
            uint16_t shiftRegister;

            bool envelopeStartFlag;
            uint8_t envelopeVolume;
            uint8_t envelopeDivider;

            static const uint8_t LENGTH_TABLE[32];
            static const uint16_t NOISE_PERIOD_TABLE[16];
    };
}