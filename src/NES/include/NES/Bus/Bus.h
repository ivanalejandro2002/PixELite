#pragma once
#include <vector>
#include <cstdint>
#include <NES/Device.h>
#include <NES/Bus/OpenBus.h>
#include <functional>

namespace NES
{
    class Device;

    class Bus
    {
        public:

            // Máscaras de direcciones
            struct AddressMask
            {
                uint16_t start;
                uint16_t end;

                uint16_t andMask = 0xFFFF;
                uint16_t orMask = 0x0000;
            };

            // Máscaras de datos
            
            struct DataMask
            {
                uint16_t start;
                uint16_t end;

                uint8_t andMask;
                uint8_t orMask;
            };

            Bus();

            uint8_t read(uint16_t addr);
            void write(uint16_t addr, uint8_t value);

            void resetMasks();

            void addDevice(Device *device);

            void addAddressMask(const AddressMask &mask);
            void addReadMask(const DataMask &mask);
            void addWriteMask(const DataMask &mask);
            void resetBus();

            OpenBus& getOpenBus();

            bool getNMILine();
            bool getIRQLine();
            bool getNMIActivation();

            void setNMILine(const bool &state);
            void setIRQLine(const bool &state);
            void finishNMI();

            void triggerOAMDMA(uint8_t page);
            void clockDMA();
            bool isDMAActive() {return dmaActive;}
            bool isDMARunning();
            void countDownDMA();
            void startDMA(uint16_t cycles);

            struct DMCHandler {
                std::function<bool()> isRequested;
                std::function<uint16_t()> getAddress;
                std::function<void(uint8_t)> deliverSample;
                std::function<void()> onFrameConflict; // Para limpiar el Frame IRQ si lee $xx15
                std::function<uint16_t()> getCpuAddress;
            };

            void setDMCHandler(DMCHandler handler) { dmcHandler = handler; }
            bool isDMCRunning() const { return dmcState != DMCDMAState::Idle; }
            void clockDMCDMA();

            uint64_t totalCpuCycles = 0;
        private:
            std::vector<Device *> devices;
            std::vector<AddressMask> addressMasks;

            std::vector<DataMask> readMasks;
            std::vector<DataMask> writeMasks;

            OpenBus openBus;

            DMCHandler dmcHandler;
            enum class DMCDMAState {
                Idle,
                Halt,
                Dummy,
                Alignment,
                Get,
            };

            DMCDMAState dmcState = DMCDMAState::Idle;
            uint16_t dmcReadAddress = 0;

            uint16_t normalizeAddress(uint16_t address) const;
            uint8_t applyReadMasks(uint16_t addr, uint8_t value)const;
            uint8_t applyWriteMasks(uint16_t addr, uint8_t value)const;

            bool NMILine;
            bool IRQLine;
            bool NMIActivation;

            bool DMAInProgress;

            bool dmaActive = false;
            bool dmaDummy = true;
            uint8_t dmaPage = 0x00;
            uint8_t dmaAddr = 0x00;
            uint8_t dmaData = 0x00;
            uint16_t remainingDMACycles;

            uint16_t cpuLastReadAddress = 0x0000;
    };
}