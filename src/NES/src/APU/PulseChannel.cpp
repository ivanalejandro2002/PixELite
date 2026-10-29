#include <NES/APU/PulseChannel.h>

namespace NES
{
    const uint8_t PulseChannel::LENGTH_TABLE[32] = {
        10, 254, 20,  2,  40,  4,  80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
        12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
    };

    static const uint8_t DUTY_TABLE[4][8] = {
        {0, 1, 0, 0, 0, 0, 0, 0}, // 12.5%
        {0, 1, 1, 0, 0, 0, 0, 0}, // 25.0%
        {0, 1, 1, 1, 1, 0, 0, 0}, // 50.0%
        {1, 0, 0, 1, 1, 1, 1, 1}  // 75.0% (negado de 25%)
    };

    PulseChannel::PulseChannel(bool isPulse1) : isPulse1(isPulse1)
    {
        enabled = false;
        dutyCycle = 0;
        lengthCounterEnable = false;
        constantVolume = false;
        volumeOrEnvelopeReload = 0;
        timerReload = 0;
        timerCounter = 0;
        sequencerStep = 0;
        lengthCounter = 0;
        envelopeStartFlag = false;
        envelopeVolume = 0;
        envelopeDivider = 0;
        sweepEnabled = false;
        sweepReloadFlag = false;
        sweepNegate = false;
        sweepShift = 0;
        sweepDivider = 0;
    }

    uint8_t PulseChannel::getLengthCounter() const {return lengthCounter;}

    void PulseChannel::setEnabled(bool enable)
    {
        enabled = enable;
        if(!enable)
        {
            lengthCounter = 0;
        }
    }

    void PulseChannel::writeControl(uint8_t value)
    {
        dutyCycle = (value >> 6) & 0x03;
        lengthCounterEnable = (value & 0x20) != 0;
        constantVolume = (value & 0x10) != 0;
        volumeOrEnvelopeReload = value & 0x0F;
    }

    void PulseChannel::writeSweep(uint8_t value)
    {
        sweepEnabled = (value & 0x80) != 0;
        sweepDivider = ((value >> 4) & 0x07) + 1;
        sweepNegate = (value & 0x08) != 0;
        sweepShift = value & 0x07;
        sweepReloadFlag = true;
    }

    void PulseChannel::writeTimerLow(uint8_t value)
    {
        timerReload = (timerReload & 0xFF00) | value;
    }

    void PulseChannel::writeTimerHigh(uint8_t value)
    {
        timerReload = (timerReload & 0x00FF) | (static_cast<uint16_t>(value & 0x07) << 8);

        if(enabled)
        {
            lengthCounter = LENGTH_TABLE[(value >> 3) & 0x1F];
        }

        sequencerStep = 0;
        envelopeStartFlag = true;
    }

    void PulseChannel::clockTimer()
    {
        if(timerCounter == 0)
        {
            timerCounter = timerReload;
            sequencerStep = (sequencerStep + 1) % 8;
        }
        else
        {
            timerCounter--;
        }
    }

    uint8_t PulseChannel::output() const
    {
        if(!enabled || lengthCounter == 0) return 0;

        if(timerReload < 8) return 0;

        uint8_t wave = DUTY_TABLE[dutyCycle][sequencerStep];
        if(wave == 0) return 0;

        return constantVolume? volumeOrEnvelopeReload : envelopeVolume;
    }

    void PulseChannel::clockLengthCounter()
    {
        if(!lengthCounterEnable && lengthCounter > 0)
        {
            lengthCounter--;
        }
    }

    void PulseChannel::clockEnvelope()
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
                    // Modo loop
                    envelopeVolume = 15;
                }
            }
        }
    }

    void PulseChannel::clockSweep()
    {
        // Barrido de frecuencia
        if(sweepDivider > 0)
        {
            sweepDivider--;
        }
        else
        {
            sweepDivider = ((sweepDivider >> 4) & 0x07) + 1;
            if(sweepEnabled && sweepShift > 0 && timerReload >= 8)
            {
                uint16_t change = timerReload >> sweepShift;
                if(sweepNegate)
                {
                    // Pulse1 tiene un complemento a 1 implícito
                    timerReload -= change;
                }
                else
                {
                    timerReload += change;
                }
            }
        }

        if(sweepReloadFlag)
        {
            sweepReloadFlag = false;
            sweepDivider = ((sweepDivider >> 4) & 0x07) + 1;
        }
    }
}