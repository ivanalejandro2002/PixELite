#include <NES/APU/DMCChannel.h>
#include <NES/Bus/Bus.h>

namespace NES
{
    const uint16_t DMCChannel::NTSC_RATE_TABLE[16] = {
        428, 380, 340, 320, 286, 253, 224, 254, 203, 175, 158, 128, 110, 80, 50, 36
    };

    DMCChannel::DMCChannel(Bus &bus) : busReference(bus)
    {
        enabled = false;
        irqEnabled = false;
        loopFlag = false;
        rateIndex = 0;
        outputLevel = 0;
        sampleAddressReload = 0;
        sampleLengthReload = 0;
        currentAddress = 0;
        bytesRemaining = 0;
        sampleBuffer = 0;
        bufferEmpty = true;
        shiftRegister = 0;
        bitsRemaining = 0;
        silenceFlag = true;
        timerReload = NTSC_RATE_TABLE[0];
        timerCounter = 0;
        irqFlag = false;
    }

    void DMCChannel::setEnabled(bool enable)
    {
        enabled = enable;
        if(!enabled)
        {
            bytesRemaining = 0;
        }
        else
        {
            if(bytesRemaining == 0)
            {
                startSample();
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
        irqEnabled = (value & 0x40) != 0;
        loopFlag = (value & 0x20) != 0;
        rateIndex = value & 0x0F;
        timerReload = NTSC_RATE_TABLE[rateIndex];

        if(irqEnabled)
        {
            irqFlag = false;
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
        if(timerCounter == 0)
        {
            timerCounter = timerReload;

            // Avance de registro de desplazamiento del DMC
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
                }
            }
        }
        else
        {
            timerCounter--;
        }

        // Buffer vacío y quedan bytes por leer
        if(bufferEmpty && bytesRemaining > 0)
        {
            // Lectura de memoria con simulación de robo de ciclos
            sampleBuffer = busReference.read(currentAddress);
            bufferEmpty = false;

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
                else if(irqEnabled)
                {
                    irqFlag = true;
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
    }

    uint16_t DMCChannel::getBytesRemaining() const {return bytesRemaining;}
}