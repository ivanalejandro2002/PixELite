#pragma once
#include <cstdint>

namespace NES
{
    class Bus;

    class DMCChannel
    {
        public:
            DMCChannel(Bus &bus);
            ~DMCChannel() = default;

            void writeFlagsRate(uint8_t value);
            void writeDirectLoad(uint8_t value);
            void writeSampleAddress(uint8_t value);
            void writeSampleLength(uint8_t value);

            void clockTimer();
            void setEnabled(bool enable);
            bool isEnabled() const;

            bool isAbortedDMARequested() const { return abortedDmaRequested; }
            void clearAbortedDMARequest() { abortedDmaRequested = false; }

            uint8_t output() const;
            bool getIRQFlag() const;
            void clearIRQFlag();

            bool isDMARequested() const {return dmaRequested;}
            uint16_t getDMAAddress() const {return currentAddress;}
            void deliverDMASample(uint8_t data);
            void cancelDMARequest() {dmaRequested = false;}

            uint16_t getBytesRemaining() const;
            Bus& getBusReference();

        private:
            Bus &busReference;
            bool enabled;

            bool irqEnabled;
            bool loopFlag;
            uint16_t rateIndex;

            uint8_t outputLevel;

            uint16_t sampleAddressReload;
            uint16_t sampleLengthReload;

            uint16_t currentAddress;
            uint16_t bytesRemaining;
            uint8_t sampleBuffer;
            bool bufferEmpty;
            uint8_t shiftRegister;
            uint8_t bitsRemaining;
            bool silenceFlag;

            uint16_t timerReload;
            uint16_t timerCounter;

            bool irqFlag;

            bool dmaRequested = false;
            uint8_t loadDmaDelay = 0;
            uint8_t abortedDmaDelay = 0;
            bool abortedDmaRequested = false;

            bool pendingAbortedDMA = false;
            bool pendingFullReloadDMA = false;
            
            void startSample();

            static const uint16_t NTSC_RATE_TABLE[16];
    };
}