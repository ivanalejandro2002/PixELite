#include <NES/APU/NoiseChannel.h>

namespace NES
{
    const uint8_t NoiseChannel::LENGTH_TABLE[32] = {
        10, 254, 20,  2,  40,  4,  80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
        12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
    };

    const uint16_t NoiseChannel::NOISE_PERIOD_TABLE[16] = {
        4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 1016, 2034, 4068, 8136
    };

    NoiseChannel::NoiseChannel()
    {
        enabled = false;
        lengthCounterEnable = false;
        constantVolume = false;
        volumeOrEnvelopeReload = 0;
        modeFlag = false;
        timerReload = 0;
        lengthCounter = 0;
        timerCounter = 0;
        shiftRegister = 1;
        envelopeStartFlag = false;
        envelopeVolume = 0;
        envelopeDivider = 0;
    }

    uint8_t NoiseChannel::getLengthCounter() const {return lengthCounter;}

    void NoiseChannel::setEnabled(bool enable)
    {
        enabled = enable;
        if(!enable)
        {
            lengthCounter = 0;
        }
    }

    void NoiseChannel::writeControl(uint8_t value)
    {
        lengthCounterEnable = (value & 0x20) != 0;
        constantVolume = (value & 0x10) != 0;
        volumeOrEnvelopeReload = value & 0x0F;
    }

    void NoiseChannel::writePeriod(uint8_t value)
    {
        modeFlag = (value & 0x80) != 0;
        timerReload = NOISE_PERIOD_TABLE[value & 0x0F];
    }

    void NoiseChannel::writeLength(uint8_t value)
    {
        if(enabled)
        {
            lengthCounter = LENGTH_TABLE[(value >> 3) & 0x1F];
        }
        envelopeStartFlag = true;
    }

    void NoiseChannel::clockTimer()
    {
        if(timerCounter == 0)
        {
            timerCounter = timerReload;

            // Cálculo LFSR (Bit 0 XOR bit 1) si modeFlag = true
            // (bit 0 XOR bit 6) si modeFlag = false
            uint8_t feedbackBit = modeFlag ? 1 : 6;
            uint16_t bit0 = shiftRegister & 0x0001;
            uint16_t bitFeedback = (shiftRegister >> feedbackBit) & 0x0001;
            uint16_t result = bit0 ^ bitFeedback;

            shiftRegister >>= 1;
            shiftRegister |= (result << 14);
        }
        else
        {
            timerCounter--;
        }
    }

    void NoiseChannel::clockEnvelope()
    {
        if(envelopeStartFlag)
        {
            envelopeStartFlag = false;
            envelopeVolume = 15;
            envelopeDivider = volumeOrEnvelopeReload;
        }
        else
        {
            if(envelopeDivider > 0)
            {
                envelopeDivider--;
            }
            else
            {
                envelopeDivider = volumeOrEnvelopeReload;
                if(envelopeVolume > 0)
                {
                    envelopeVolume--;
                }
                else if(lengthCounterEnable)
                {
                    envelopeVolume = 15;
                }
            }
        }
    }

    void NoiseChannel::clockLengthCounter()
    {
        if(!lengthCounterEnable && lengthCounter > 0)
        {
            lengthCounter--;
        }
    }
    
    uint8_t NoiseChannel::output() const
    {
        // Si el canal no está activo, la longitud expiró o el bit 0 de LFSR, la salida es 0
        if(!enabled || lengthCounter == 0 || (shiftRegister & 0x0001) != 0)
        {
            return 0;
        }
        return constantVolume ? volumeOrEnvelopeReload : envelopeVolume;
    }
}