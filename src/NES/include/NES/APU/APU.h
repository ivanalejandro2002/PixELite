#pragma once

#include <NES/APU/PulseChannel.h>
#include <NES/APU/TriangleChannel.h>
#include <NES/APU/NoiseChannel.h>
#include <NES/APU/DMCChannel.h>
#include <NES/APU/AudioRingBuffer.h>

#include <NES/Bus/Bus.h>

#include <NES/Device.h>
#include <cstdint>

namespace NES
{
    class APU : public Device
    {
        public:
            APU(Bus &bus);
            ~APU() = default;

            bool handles(uint16_t addr) const override;
            uint8_t read(uint16_t addr) override;
            void write(uint16_t addr, uint8_t value) override;

            void clock();

            double getAudioSample();

            double getTestToneSample();
            bool testMode = false;
            void activateTestMode();
            void deactivateTestMode();

        private:
            PulseChannel pulse1{true};
            PulseChannel pulse2{false};

            TriangleChannel triangle;

            NoiseChannel noise;

            DMCChannel dmc;

            uint8_t statusRegister;

            uint32_t cpuCycleCount;
            bool fiveStepMode;
            bool irqInhibit;
            uint8_t frameStep;

            void handleFrameCounterWrite(uint8_t value);
    };
}