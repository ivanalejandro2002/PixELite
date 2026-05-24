#pragma once
#include <vector>
#include <cstdint>
#include <NES/Device.h>
#include <NES/Bus/OpenBus.h>

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

            uint8_t read(uint16_t addr);
            void write(uint16_t addr, uint8_t value);

            void addBusMask(uint16_t start, uint16_t end, uint8_t andMask, uint8_t orMask);
            void resetMasks();

            uint16_t normalizeAddress();

            void addDevice(Device *device);

            void addAddressMask(const AddressMask &mask);
            void addReadMask(const DataMask &mask);
            void addWriteMask(const DataMask &mask);
        private:
            std::vector<Device *> devices;
            std::vector<AddressMask> addressMasks;

            std::vector<DataMask> readMasks;
            std::vector<DataMask> writeMasks;

            OpenBus openBus;

            uint16_t normalizeAddress(uint16_t address) const;
            uint8_t applyReadMasks(uint16_t addr, uint8_t value)const;
            uint8_t applyWriteMasks(uint16_t addr, uint8_t value)const;
    };
}