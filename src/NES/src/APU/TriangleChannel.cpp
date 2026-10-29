#include <NES/APU/TriangleChannel.h>

namespace NES
{
    const uint8_t TriangleChannel::LENGTH_TABLE[32] = {
        10, 254, 20,  2,  40,  4,  80,  6, 160,  8, 60, 10, 14, 12, 26, 14,
        12,  16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
    };

    const uint8_t TriangleChannel::TRIANGLE_SEQUENCE[32] = {
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
         0,  1,  2,  3,  4,  5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };

    TriangleChannel::TriangleChannel()
    {
        enabled = false;
        controlFlag = false;
        linearCounterReloadValue = 0;
        timerReload = 0;
        timerCounter = 0;
        sequencerStep = 0;
        linearCounter = 0;
        linearcounterReloadFlag = false;
        lengthCounter = 0;
    }

    uint8_t TriangleChannel::getLengthCounter() const {return lengthCounter;}

    void TriangleChannel::setEnabled(bool enable)
    {
        enabled = enable;
        if(!enable)
        {
            lengthCounter = 0;
        }
    }

    void TriangleChannel::writeControl(uint8_t value)
    {
        controlFlag = (value & 0x80) != 0;
        linearCounterReloadValue = value & 0x7F;
    }

    void TriangleChannel::writeTimerLow(uint8_t value)
    {
        timerReload = (timerReload & 0xFF00) | value;
    }

    void TriangleChannel::writeTimerHigh(uint8_t value)
    {
        timerReload = (timerReload & 0x00FF) | (static_cast<uint16_t>(value & 0x07) << 8);

        if(enabled)
        {
            lengthCounter = LENGTH_TABLE[(value >> 3) & 0x1F];
        }
        linearcounterReloadFlag = true;
    }

    void TriangleChannel::clockTimer()
    {
        // Marca un tick cada ciclo de reloj
        if(timerCounter == 0)
        {
            timerCounter = timerReload;

            //Solo avanza si ambos contadores son mayores a 0
            if(linearCounter > 0 && lengthCounter > 0)
            {
                sequencerStep = (sequencerStep + 1) % 32;
            }
        }
        else
        {
            timerCounter--;
        }
    }

    void TriangleChannel::clockLinearCounter()
    {
        if(linearcounterReloadFlag)
        {
            linearCounter = linearCounterReloadValue;
        }
        else if(linearCounter > 0)
        {
            linearCounter--;
        }

        if(!controlFlag)
        {
            linearcounterReloadFlag = false;
        }
    }

    void TriangleChannel::clockLengthCounter()
    {
        if(!controlFlag && lengthCounter > 0)
        {
            lengthCounter--;
        }
    }

    uint8_t TriangleChannel::output() const
    {
        if(!enabled || lengthCounter == 0 || linearCounter == 0) return 0;
        return TRIANGLE_SEQUENCE[sequencerStep];
    }
}