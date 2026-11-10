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
        oamNeedRealign = false;
    }

    void Bus::clockDMA() {
        if (!dmaActive) return;

        uint16_t targetAddr = dmcHandler.getCpuAddress ? dmcHandler.getCpuAddress() : cpuLastReadAddress;

        if (dmaDummy) {
            // Ciclo Halt / Dummy inicial de OAM DMA: la CPU repite la lectura en el bus
            this->read(targetAddr);
            if (totalCpuCycles % 2 == 1) {
                dmaDummy = false;
            }
        } else if (oamNeedRealign) {
            // Ciclo PUT inmediatamente posterior a que Get de DMC DMA pausó a OAM DMA:
            // OAM DMA no puede hacer Get en ciclo PUT, así que se realinea leyendo del bus
            this->read(targetAddr);
            oamNeedRealign = false;
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
                // 1er ciclo (Halt):
                if (dmaActive) {
                    clockDMA();
                } else {
                    uint16_t targetAddr = dmcHandler.getCpuAddress ? dmcHandler.getCpuAddress() : cpuLastReadAddress;
                    this->read(targetAddr);
                }
                dmcState = DMCDMAState::Dummy;
            } else if (dmcHandler.isAbortedRequested && dmcHandler.isAbortedRequested()) {
                dmcHandler.clearAbortedRequest();
                // Aborted DMA de 1 solo ciclo:
                if (dmaActive) {
                    clockDMA();
                } else {
                    uint16_t targetAddr = dmcHandler.getCpuAddress ? dmcHandler.getCpuAddress() : cpuLastReadAddress;
                    this->read(targetAddr);
                }
                dmcState = DMCDMAState::Idle;
            }
            return;
        }

        switch (dmcState)
        {
            case DMCDMAState::Dummy:
            {
                // 2do ciclo (Dummy):
                if (dmaActive) {
                    clockDMA();
                } else {
                    uint16_t targetAddr = dmcHandler.getCpuAddress ? dmcHandler.getCpuAddress() : cpuLastReadAddress;
                    this->read(targetAddr);
                }

                if ((totalCpuCycles & 1) == 0) {
                    dmcState = DMCDMAState::Alignment;
                } else {
                    dmcState = DMCDMAState::Get;
                }
                break;
            }

            case DMCDMAState::Alignment:
            {
                // 3er ciclo opcional (Alignment):
                if (dmaActive) {
                    clockDMA();
                } else {
                    uint16_t targetAddr = dmcHandler.getCpuAddress ? dmcHandler.getCpuAddress() : cpuLastReadAddress;
                    this->read(targetAddr);
                }
                dmcState = DMCDMAState::Get;
                break;
            }

            case DMCDMAState::Get:
            {
                // Si OAM DMA estaba transfiriendo bytes activamente, pierde este ciclo GET
                // y se realinea en el siguiente PUT:
                if (dmaActive && !dmaDummy) {
                    oamNeedRealign = true;
                } else if (dmaActive && dmaDummy) {
                    if (totalCpuCycles % 2 == 1) {
                        dmaDummy = false;
                    }
                }

                uint16_t currentCpuAddr = dmcHandler.getCpuAddress ? dmcHandler.getCpuAddress() : cpuLastReadAddress;

                // Decodificación de direcciones del 2A03:
                // bits 15-5 de la CPU + bits 4-0 de la dirección del DMA
                uint16_t combinedAddr = (currentCpuAddr & 0xFFE0) | (dmcReadAddress & 0x001F);

                uint8_t sample = 0;

                if (combinedAddr == 0x4015) {
                    // Conflicto con $4015:
                    // El cartucho coloca romByte en el bus externo (actualiza openBus)
                    // y el 2A03 lee $4015 internamente (limpia Frame IRQ):
                    uint8_t romByte = this->read(dmcReadAddress);
                    uint8_t statusByte = this->read(0x4015);
                    sample = (statusByte & 0xDF) | (romByte & 0x20);
                    openBus.update(romByte);
                } else if (combinedAddr == 0x4016 || combinedAddr == 0x4017) {
                    // Conflicto con puertos de control:
                    uint8_t romByte = this->read(dmcReadAddress);
                    uint8_t joyByte = this->read(combinedAddr);
                    sample = romByte & joyByte;
                    openBus.update(sample);
                } else {
                    sample = this->read(dmcReadAddress);
                    openBus.update(sample);
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

        // $4014 (OAMDMA) es solo de escritura; leerlo debe devolver Open Bus
        // (evita que PPURegisters lo mapee por error a $2004 OAMDATA)
        if(addr == 0x4014)
        {
            return openBus.getValue();
        }

        for(Device *device: devices)
        {
            if(device -> handlesRead(addr))
            {
                uint8_t value = device -> read(addr);

                // Los bits 5, 6 y 7 de $4016 y $4017 provienen del open bus
                if(addr == 0x4016 || addr == 0x4017)
                {
                    value = (value & 0x1F) | (openBus.getValue() & 0xE0);
                }

                // En $4015, el bit 5 es Open Bus y la lectura es interna al 2A03
                // (NO actualiza el estado de openBus)
                if(addr == 0x4015)
                {
                    value = (value & 0xDF) | (openBus.getValue() & 0x20);
                    value = applyReadMasks(addr, value);
                    return value;
                }

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
            if(device -> handlesWrite(addr))
            {
                device -> write(addr, value);
                return;
            }
        }
    }
}