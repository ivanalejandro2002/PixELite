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
    }

    void DMCChannel::setEnabled(bool enable)
    {
        enabled = enable;
        if(!enabled)
        {
            // Explicit DMA Abort: si una muestra estaba activa y se deshabilita
            // cuando faltan 2 o menos ciclos para el reload del timer:
            if(bytesRemaining > 0 && bitsRemaining == 1 && timerCounter <= 2)
            {
                dmaRequested = true; // El 2A03 ya agendó el halt internamente
            }
            else
            {
                dmaRequested = false;
            }

            bytesRemaining = 0;
            loadDmaDelay = 0;
        }
        else
        {
            if(bytesRemaining == 0)
            {
                startSample();

                if(bufferEmpty && bytesRemaining > 0)
                {
                    // Load DMA: +4 ciclos si es ciclo par (GET), +3 si es impar (PUT)
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
                if(bufferEmpty)
                {
                    silenceFlag = true;
                }
                else
                {
                    silenceFlag = false;
                    shiftRegister = sampleBuffer;
                    bufferEmpty = true;

                    // Reload DMA: solicita DMA inmediatamente al vaciar el buffer
                    if(bytesRemaining > 0)
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

        currentAddress++;
        if(currentAddress == 0x0000)
        {
            currentAddress = 0x8000;
        }

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

                // Implicit DMA Abort: la muestra terminó justo en el ciclo de APU previo a un Reload DMA:
                if(bitsRemaining == 1 && timerCounter <= 2)
                {
                    dmaRequested = true;
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