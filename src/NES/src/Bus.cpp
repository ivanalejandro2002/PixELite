#include <NES/Bus/Bus.h>
#include <random>
#include <cstdint>
#include <iostream>

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

    Bus::Bus()
    {
        NMILine = IRQLine = true;
        NMIActivation = false;

        DMAInProgress = false;
        remainingDMACycles = 0;
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

    void Bus::resetBus()
    {
        NMILine = IRQLine = 1;
        NMIActivation = 0;

        DMAInProgress = false;
        remainingDMACycles = 0;
    }

    OpenBus& Bus::getOpenBus()
    {
        return openBus;
    }

    bool Bus::getNMILine()
    {
        return NMILine;
    }

    bool Bus::getIRQLine()
    {
        return IRQLine;
    }

    bool Bus::getNMIActivation()
    {
        return NMIActivation;
    }

    void Bus::setNMILine(const bool &status)
    {
        if(!status && NMILine)
        {
            NMIActivation = true;
        }
        NMILine = status;
    }

    void Bus::setIRQLine(const bool &status)
    {
        IRQLine = status;
    }

    void Bus::finishNMI()
    {
        NMIActivation = 0;
    }

    void Bus::triggerOAMDMA(uint8_t page) {
        dmaActive = true;
        dmaDummy = true;
        dmaPage = page;
        dmaAddr = 0x00;
        dmaData = 0x00;
    }

    void Bus::clockDMA() {
        if (!dmaActive) return;

        if (dmaDummy) {
            // En ciclos impares de CPU se requiere esperar a un ciclo par
            if (totalCpuCycles % 2 == 1) {
                dmaDummy = false;
            }
        } else {
            // Ciclos de transferencia alternados
            if (totalCpuCycles % 2 == 0) {
                // Ciclo de LECTURA (Get)
                uint16_t readAddr = (static_cast<uint16_t>(dmaPage) << 8) | dmaAddr;
                dmaData = this->read(readAddr);
            } else {
                // Ciclo de ESCRITURA (Put) en $2004 (OAMDATA)
                this->write(0x2004, dmaData);
                dmaAddr++;

                if (dmaAddr == 0) {
                    // Al desbordar de 255 a 0, la transferencia de 256 bytes ha finalizado
                    dmaActive = false;
                    dmaDummy = true;
                }
            }
        }
    }

    void Bus::clockDMCDMA()
    {
        if (dmcState == DMCDMAState::Idle) {
            if (dmcHandler.isRequested && dmcHandler.isRequested()) {
                dmcReadAddress = dmcHandler.getAddress();
                // 1er ciclo (Halt): La CPU se detiene y repite la lectura en el bus
                this->read(cpuLastReadAddress);
                // El ciclo 1 ya fue Halt, así que el ciclo 2 será Dummy:
                dmcState = DMCDMAState::Dummy;
            }
            return; // Consume este ciclo de CPU y termina
        }

        switch (dmcState)
        {
            case DMCDMAState::Dummy:
                // 2do ciclo (Dummy): La CPU vuelve a repetir la lectura en el bus
                this->read(cpuLastReadAddress);

                // Si el ciclo actual es par, el siguiente es impar (Put) -> requiere Alignment
                // Si el ciclo actual es impar, el siguiente es par (Get) -> va directo a Get
                if ((totalCpuCycles & 1) == 0) {
                    dmcState = DMCDMAState::Alignment;
                } else {
                    dmcState = DMCDMAState::Get;
                }
                break;

            case DMCDMAState::Alignment:
                // 3er ciclo opcional (Alignment): La CPU vuelve a repetir la lectura en el bus
                this->read(cpuLastReadAddress);
                dmcState = DMCDMAState::Get;
                break;

            case DMCDMAState::Get:
            {
                // 4to ciclo (Get): El DMA toma el bus y lee la muestra de la ROM
                uint8_t sample = this->read(dmcReadAddress);

                // Refleja la muestra en Open Bus
                openBus.update(sample);

                // Conflicto de bus del 2A03: si termina en $15, limpia Frame IRQ
                if ((dmcReadAddress & 0x1F) == 0x15 && dmcHandler.onFrameConflict) {
                    dmcHandler.onFrameConflict();
                }

                if (dmcHandler.deliverSample) {
                    dmcHandler.deliverSample(sample);
                }

                dmcState = DMCDMAState::Idle;
                break;
            }

            default:
                dmcState = DMCDMAState::Idle;
                break;
        }
    }

    bool Bus::isDMARunning() {return DMAInProgress;}
    
    void Bus::countDownDMA()
    {
        remainingDMACycles --;
        if(remainingDMACycles == 0)
        {
            DMAInProgress = false;
        }
    }

    void Bus::startDMA(uint16_t _cycles)
    {
        DMAInProgress = true;
        remainingDMACycles = _cycles;
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
        if(dmcState != DMCDMAState::Get)
        {
            cpuLastReadAddress = addr;
        }
        addr = normalizeAddress(addr);

        bool hubo = 0;

        for(Device *device: devices)
        {
            if(device -> handlesRead(addr))
            {
                hubo = 1;
                uint8_t value = device -> read(addr);

                value = applyReadMasks(addr, value);

                openBus.update(value);

                return value;
            }
        }

        if(!hubo) std::cout << "NO hubo un device para la dirección: " << addr << "\n";
        return openBus.getValue();
    }

    void Bus::write(uint16_t addr, uint8_t value)
    {
        addr = normalizeAddress(addr);

        value = applyWriteMasks(addr, value);

        openBus.update(value);

        for(Device *device: devices)
        {
            if(device -> handlesWrite(addr))
            {
                device -> write(addr, value);
                return;
            }
        }
    }
}