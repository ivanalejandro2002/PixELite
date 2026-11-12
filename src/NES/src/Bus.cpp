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
        NMILine = true;
        IRQLine = true;
        NMIActivation = false;

        DMAInProgress = false;
        remainingDMACycles = 0;

        dmaActive = false;
        dmaDummy = true;
        dmaPage = 0;
        dmaAddr = 0;
        dmaData = 0;
        oamNeedRealign = false;

        dmcState = DMCDMAState::Idle;
        dmcReadAddress = 0;
        cpuLastReadAddress = 0;

        totalCpuCycles = 0;
        lastControllerReadCycle[0] = 0;
        lastControllerReadCycle[1] = 0;
        lastControllerValue[0] = 0;
        lastControllerValue[1] = 0;
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
        for (const Device* device : devices) {
            if (device->hasPendingIRQ()) {
                return false; // Activo en bajo: el cartucho/dispositivo solicita IRQ
            }
        }
        return IRQLine; // Estado reportado por la APU
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
            this->read(targetAddr);
            oamNeedRealign = false;
        } else {
            // Ciclos de transferencia alternados
            if (totalCpuCycles % 2 == 0) {
                // Ciclo de LECTURA (Get)
                uint16_t readAddr = (static_cast<uint16_t>(dmaPage) << 8) | dmaAddr;
                bool cpuInApuRange = (targetAddr & 0xFFE0) == 0x4000;
                bool isApuRegAddr = (readAddr >= 0x4000 && readAddr <= 0x401F);

                if (!cpuInApuRange) {
                    // Si la CPU no está en $4000-$401F, OAM DMA no puede activar $4015-$4017
                    if (readAddr >= 0x4015 && readAddr <= 0x4017) {
                        dmaData = openBus.getValue();
                    } else {
                        dmaData = this->read(readAddr);
                    }
                } else {
                    // La CPU está en $4000-$401F: los bits 4-0 de readAddr activan registros de la APU
                    uint8_t regOffset = readAddr & 0x001F;

                    if (regOffset == 0x15) {
                        uint8_t extVal = isApuRegAddr ? openBus.getValue() : this->read(readAddr);
                        openBus.update(extVal);
                        uint8_t statusByte = this->read(0x4015);
                        dmaData = (statusByte & 0xDF) | (extVal & 0x20);
                    } else if (regOffset == 0x16 || regOffset == 0x17) {
                        uint8_t extVal = isApuRegAddr ? openBus.getValue() : this->read(readAddr);
                        openBus.update(extVal);
                        uint8_t joyByte = this->read(0x4000 | regOffset);
                        // Si readAddr apunta a memoria activa (RAM $0000-$1FFF, PPU $2000-$3FFF o ROM >= $8000),
                        // ocurre un conflicto de bus (AND) entre extVal y joyByte:
                        bool isDrivenMemory = (readAddr < 0x4000) || (readAddr >= 0x8000);
                        dmaData = isDrivenMemory ? (extVal & joyByte) : joyByte;
                        openBus.update(dmaData);
                    } else {
                        dmaData = isApuRegAddr ? openBus.getValue() : this->read(readAddr);
                    }
                }
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

                // DMA abortado de 1 solo ciclo:
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
                if (dmaActive && !dmaDummy) {
                    oamNeedRealign = true;
                } else if (dmaActive && dmaDummy) {
                    if (totalCpuCycles % 2 == 1) {
                        dmaDummy = false;
                    }
                }

                uint16_t currentCpuAddr = dmcHandler.getCpuAddress ? dmcHandler.getCpuAddress() : cpuLastReadAddress;
                uint16_t combinedAddr = (currentCpuAddr & 0xFFE0) | (dmcReadAddress & 0x001F);
                uint8_t sample = 0;

                if (combinedAddr == 0x4015) {
                    uint8_t romByte = this->read(dmcReadAddress);
                    uint8_t statusByte = this->read(0x4015);
                    sample = (statusByte & 0xDF) | (romByte & 0x20);
                    openBus.update(romByte);
                } else if (combinedAddr == 0x4016 || combinedAddr == 0x4017) {
                    // 1. La lectura de la ROM coloca romByte en openBus (bits 7-5)
                    uint8_t romByte = this->read(dmcReadAddress);
                    openBus.update(romByte);
                    // 2. El buffer del puerto de mandos conduce los bits 4-0 y conserva los bits 7-5 de openBus (romByte)
                    uint8_t joyByte = this->read(combinedAddr);
                    sample = joyByte;
                    openBus.update(joyByte);
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
        if(!dmaActive && dmcState == DMCDMAState::Idle)
        {
            cpuLastReadAddress = addr;
        }
        addr = normalizeAddress(addr);

        if(addr == 0x4014)
        {
            return openBus.getValue();
        }

        for(Device *device: devices)
        {
            if(device -> handlesRead(addr))
            {
                uint8_t value = 0;

                // Filtrado de lecturas en ciclos consecutivos para $4016 y $4017:
                if(addr == 0x4016 || addr == 0x4017)
                {
                    int port = (addr == 0x4017) ? 1 : 0;
                    if(totalCpuCycles == lastControllerReadCycle[port] + 1)
                    {
                        value = lastControllerValue[port];
                    }
                    else
                    {
                        value = device -> read(addr);
                        lastControllerValue[port] = value;
                    }
                    lastControllerReadCycle[port] = totalCpuCycles;
                    value = (value & 0x1F) | (openBus.getValue() & 0xE0);
                }
                else if(addr == 0x4015)
                {
                    value = device -> read(addr);
                    value = (value & 0xDF) | (openBus.getValue() & 0x20);
                    value = applyReadMasks(addr, value);
                    return value;
                }
                else
                {
                    value = device -> read(addr);
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