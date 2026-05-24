#include <NES/Bus/Bus.h>
#include <random>
#include <cstdint>

namespace NES
{
    OpenBus::OpenBus()
    {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_int_distribution<uint16_t> distrib(0,255);

        lastValue = static_cast<uint8_t>(distrib(gen));
    }

    uint8_t OpenBus::getValue()
    {
        return lastValue;
    }

    void OpenBus::update(uint8_t value)
    {
        lastValue = value;
    }

    void Bus::addDevice(Device *device)
    {
        devices.push_back(device);
    }

    void Bus::addAddressMask(const AddressMask &mask)
    {
        addressMasks.push_back(mask);
    }

    void Bus::addReadMask(const DataMask &mask)
    {
        readMasks.push_back(mask);
    }

    void Bus::addWriteMask(const DataMask &mask)
    {
        writeMasks.push_back(mask);
    }



    uint16_t Bus::normalizeAddress(uint16_t addr) const
    {
        for(const AddressMask &mask: addressMasks)
        {
            if(addr >= mask.start && addr <= mask.end){
                addr &= mask.andMask;
                addr |= mask.orMask;
            }
        }
        return addr;
    }

    uint8_t Bus::applyReadMasks(uint16_t addr, uint8_t value)const
    {
        for(const DataMask &mask: readMasks)
        {
            if(addr >= mask.start && addr <= mask.end)
            {
                value &= mask.andMask;
                value |= mask.orMask;
            }
        }

        return value;
    }

    uint8_t Bus::applyWriteMasks(uint16_t addr, uint8_t value)const
    {
        for(const DataMask &mask: writeMasks)
        {
            if(addr >= mask.start && addr <= mask.end)
            {
                value &= mask.andMask;
                value |= mask.orMask;
            }
        }
        return value;
    }

    uint8_t Bus::read(uint16_t addr)
    {
        addr = normalizeAddress(addr);

        for(Device *device: devices)
        {
            if(device -> handles(addr))
            {
                uint8_t value = device -> read(addr);

                value = applyReadMasks(addr, value);

                openBus.update(value);

                return value;
            }
        }
        return openBus.getValue();
    }

    void Bus::write(uint16_t addr, uint8_t value)
    {
        addr = normalizeAddress(addr);

        value = applyWriteMasks(addr, value);

        openBus.update(value);

        for(Device *device: devices)
        {
            if(device -> handles(addr))
            {
                device -> write(addr, value);
                return;
            }
        }
    }
}