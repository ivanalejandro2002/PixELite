#include <NES/APU/DMCChannel.h>
#include <NES/Bus/Bus.h>

namespace NES
{
    const uint16_t DMCChannel::NTSC_RATE_TABLE[16] = {
        428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
    };

    DMCChannel::DMCChannel(Bus &bus) : busReference(bus)
    {
        enabled = false;
        irqEnabled = false;
        loopFlag = false;
        rateIndex = 0;
        outputLevel = 0;
        sampleAddressReload = 0xC000;
        sampleLengthReload = 1;
        currentAddress = 0xC000;
        bytesRemaining = 0;
        sampleBuffer = 0;
        bufferEmpty = true;
        shiftRegister = 0;
        bitsRemaining = 8;
        silenceFlag = true;
        timerReload = NTSC_RATE_TABLE[0] - 1;
        timerCounter = timerReload;
        irqFlag = false;
        dmaRequested = false;
        loadDmaDelay = 0;
        pendingAbortedDMA = false;
        pendingFullReloadDMA = false;
        abortedDmaRequested = false;
    }

    void DMCChannel::setEnabled(bool enable)
    {
        enabled = enable;
        if(!enabled)
        {
            pendingAbortedDMA = false;
            pendingFullReloadDMA = false;
            abortedDmaRequested = false;

            // Si un Reload DMA NO se ha disparado aún en este ciclo, evaluamos a cuántos
            // ciclos estamos del próximo reload (cuando bitsRemaining == 1):
            if(!dmaRequested && bytesRemaining > 0 && bitsRemaining == 1)
            {
                if(timerCounter == 2 || timerCounter == 1)
                {
                    // Explicit Abort: 2 o 3 ciclos antes de T_reload -> DMA de 1 ciclo en T_reload
                    pendingAbortedDMA = true;
                }
                else if(timerCounter == 0)
                {
                    // 1 ciclo antes de T_reload (mismo ciclo de APU) -> el Reload DMA completo aún ocurre
                    pendingFullReloadDMA = true;
                }
            }

            // Si había un Load DMA contando:
            if(loadDmaDelay > 0)
            {
                if(loadDmaDelay <= 2)
                {
                    abortedDmaRequested = true;
                }
                loadDmaDelay = 0;
            }

            bytesRemaining = 0;
            // NOTA: Si dmaRequested ya era true (ej. STA $4015 escribió justo en T_reload
            // retrasando el halt por 1 ciclo de escritura), NO se pone dmaRequested = false.
        }
        else
        {
            if(bytesRemaining == 0)
            {
                startSample();
                pendingAbortedDMA = false;
                pendingFullReloadDMA = false;
                abortedDmaRequested = false;

                if(bytesRemaining > 0)
                {
                    loadDmaDelay = ((busReference.totalCpuCycles & 1) == 0) ? 4 : 3;
                }
            }
        }
        irqFlag = false;
    }

    bool DMCChannel::isEnabled() const
    {
        return enabled;
    }

    void DMCChannel::writeFlagsRate(uint8_t value)
    {
        irqEnabled = (value & 0x80) != 0; // Bit 7: IRQ Enable
        loopFlag   = (value & 0x40) != 0; // Bit 6: Loop
        rateIndex  = value & 0x0F;
        timerReload = NTSC_RATE_TABLE[rateIndex] - 1;

        if(!irqEnabled)
        {
            clearIRQFlag();
        }
    }

    void DMCChannel::writeDirectLoad(uint8_t value)
    {
        outputLevel = value & 0x7F;
    }

    void DMCChannel::writeSampleAddress(uint8_t value)
    {
        sampleAddressReload = 0xC000 + (static_cast<uint16_t>(value) << 6);
    }

    void DMCChannel::writeSampleLength(uint8_t value)
    {
        sampleLengthReload = (static_cast<uint16_t>(value) << 4) + 1;
    }

    void DMCChannel::startSample()
    {
        currentAddress = sampleAddressReload;
        bytesRemaining = sampleLengthReload;
    }

    void DMCChannel::clockTimer()
    {
        if(loadDmaDelay > 0)
        {
            loadDmaDelay--;
            if(loadDmaDelay == 0 && bufferEmpty && bytesRemaining > 0)
            {
                dmaRequested = true;
            }
        }

        if(timerCounter == 0)
        {
            timerReload = NTSC_RATE_TABLE[rateIndex] - 1;
            timerCounter = timerReload;

            if(!silenceFlag)
            {
                if((shiftRegister & 0x01) != 0)
                {
                    if(outputLevel <= 125) outputLevel += 2;
                }
                else
                {
                    if(outputLevel >= 2) outputLevel -= 2;
                }
            }

            shiftRegister >>= 1;
            bitsRemaining--;

            if(bitsRemaining == 0)
            {
                bitsRemaining = 8;

                if(pendingAbortedDMA)
                {
                    pendingAbortedDMA = false;
                    abortedDmaRequested = true;
                    if(!bufferEmpty)
                    {
                        silenceFlag = false;
                        shiftRegister = sampleBuffer;
                        bufferEmpty = true;
                    }
                    else
                    {
                        silenceFlag = true;
                    }
                }
                else if(pendingFullReloadDMA)
                {
                    pendingFullReloadDMA = false;
                    silenceFlag = false;
                    shiftRegister = sampleBuffer;
                    bufferEmpty = true;
                    dmaRequested = true;
                }
                else if(bufferEmpty)
                {
                    silenceFlag = true;
                }
                else
                {
                    silenceFlag = false;
                    shiftRegister = sampleBuffer;
                    bufferEmpty = true;

                    // Solo solicitar Reload DMA si no hay ya un Load DMA esperando sus 3 o 4 ciclos:
                    if(bytesRemaining > 0 && loadDmaDelay == 0)
                    {
                        dmaRequested = true;
                    }
                }
            }
        }
        else
        {
            timerCounter--;
        }
    }

    void DMCChannel::deliverDMASample(uint8_t data)
    {
        sampleBuffer = data;
        bufferEmpty = false;
        dmaRequested = false;

        uint16_t prevAddress = currentAddress;
        currentAddress++;
        if(currentAddress == 0x0000)
        {
            currentAddress = 0x8000;
        }

        if(bytesRemaining > 0)
        {
            bytesRemaining--;
            if(bytesRemaining == 0)
            {
                if(loopFlag)
                {
                    startSample();
                }
                else
                {
                    if(irqEnabled)
                    {
                        irqFlag = true;
                        busReference.setIRQLine(false);
                    }

                    // Implicit DMA Abort:
                    if(bitsRemaining == 1)
                    {
                        if(timerCounter == 2 || timerCounter == 1)
                        {
                            // Terminó en el ciclo de APU previo a T_reload -> DMA de 1 ciclo en T_reload
                            pendingAbortedDMA = true;
                        }
                        else if(timerCounter == 0)
                        {
                            // Terminó 1 ciclo antes de T_reload -> Reload DMA inesperado desde la misma dirección
                            currentAddress = prevAddress;
                            pendingFullReloadDMA = true;
                        }
                    }
                }
            }
        }
    }

    uint8_t DMCChannel::output() const
    {
        return outputLevel;
    }

    bool DMCChannel::getIRQFlag() const
    {
        return irqFlag;
    }

    void DMCChannel::clearIRQFlag()
    {
        irqFlag = false;
        busReference.setIRQLine(true); 
    }

    uint16_t DMCChannel::getBytesRemaining() const { return bytesRemaining; }
    Bus& DMCChannel::getBusReference() { return busReference; }
}