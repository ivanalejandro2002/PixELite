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

            bool handlesRead(uint16_t addr) const override;
            bool handlesWrite(uint16_t addr) const override;

            uint8_t read(uint16_t addr) override;
            void write(uint16_t addr, uint8_t value) override;

            void clock();

            double getAudioSample();

            double getTestToneSample();
            bool testMode = false;
            void activateTestMode();
            void deactivateTestMode();

            DMCChannel& getDMC() {return dmc;}
            const DMCChannel& getDMC() const {return dmc;}

            void clearFrameIRQ() {
                frameIrqFlag = false;
                if(!dmc.getIRQFlag())
                {
                    dmc.getBusReference().setIRQLine(true);
                }
            }

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
            bool frameIrqFlag = false;

            bool pulseDivider = false;

            bool pendingFrameCounterReset = false;
            uint8_t pendingFrameCounterValue = 0;
            int frameCounterResetDelay = 0;

            void handleFrameCounterWrite(uint8_t value);
    };
}