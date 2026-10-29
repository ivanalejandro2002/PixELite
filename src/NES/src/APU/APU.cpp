#include <NES/APU/APU.h>
#include <math.h>

namespace NES
{
    APU::APU(Bus &bus): dmc(bus)
    {
        statusRegister = 0;
        cpuCycleCount = 0;
        fiveStepMode = false;
        irqInhibit = true;
        frameStep = 0;
    }

    bool APU::handles(uint16_t addr) const
    {
        return (addr >= 0x4000 && addr <= 0x4013) ||
                (addr == 0x4015) || 
                (addr == 0x4017);
    }

    uint8_t APU::read(uint16_t addr)
    {
        if(addr == 0x4015)
        {
            uint8_t status = 0;

            if (pulse1.getLengthCounter() > 0) status |= 0x01;
            if (pulse2.getLengthCounter() > 0) status |= 0x02;
            if (triangle.getLengthCounter() > 0) status |= 0x04;
            if (noise.getLengthCounter() > 0) status |= 0x08;
            if (dmc.getBytesRemaining() > 0) status |= 0x10;

            if (dmc.getIRQFlag()) status |= 0x80;

            return status;
        }
        return 0;
    }

    void APU::write(uint16_t addr, uint8_t value)
    {
        if(addr >= 0x4000 && addr <= 0x4003)
        {
            // Pulso 1
            uint8_t offset = addr & 0x03;
            if(offset == 0) pulse1.writeControl(value);
            else if(offset == 1) pulse1.writeSweep(value);
            else if(offset == 2) pulse1.writeTimerLow(value);
            else if(offset == 3) pulse1.writeTimerHigh(value);

            return;
        }
        if(addr >= 0x4004 && addr <= 0x4007)
        {
            uint8_t offset = addr & 0x03;
            if(offset == 0) pulse2.writeControl(value);
            else if(offset == 1)pulse2.writeSweep(value);
            else if(offset == 2)pulse2.writeTimerLow(value);
            else if(offset == 3)pulse2.writeTimerHigh(value);

            return;
        }
        if(addr >= 0x4008 && addr <= 0x400B)
        {
            // Triángulo
            uint8_t offset = addr & 0x03;
            if(offset == 0) triangle.writeControl(value);
            if(offset == 2) triangle.writeTimerLow(value);
            if(offset == 3) triangle.writeTimerHigh(value);
            return;
        }
        if(addr >= 0x400C && addr <= 0x400F)
        {
            // Ruido
            uint8_t offset = addr & 0x03;
            if(offset == 0) noise.writeControl(value);
            else if(offset == 1) noise.writePeriod(value);
            else if(offset == 2) noise.writeLength(value);
            return;
        }
        if(addr >= 0x4010 && addr <= 0x4013)
        {
            uint8_t offset = addr & 0x03;
            if(offset == 0) dmc.writeFlagsRate(value);
            else if(offset == 1) dmc.writeDirectLoad(value);
            else if(offset == 2) dmc.writeSampleAddress(value);
            else if(offset == 3) dmc.writeSampleLength(value);
        }
        if(addr == 0x4015)
        {
            pulse1.setEnabled((value & 0x01) != 0);
            pulse2.setEnabled((value & 0x02) != 0);
            triangle.setEnabled((value & 0x04) != 0);
            noise.setEnabled((value & 0x08) != 0);
            dmc.setEnabled((value & 0x10) != 0);
            dmc.clearIRQFlag();
            statusRegister = value;
            return;
        }
        if(addr == 0x4017)
        {
            handleFrameCounterWrite(value);
            return;
        }
    }

    void APU::handleFrameCounterWrite(uint8_t value)
    {
        fiveStepMode = (value & 0x80) != 0;
        irqInhibit = (value & 0x40) != 0;

        if(irqInhibit)
        {

        }

        cpuCycleCount = 0;
        frameStep = 0;

        if(fiveStepMode)
        {

        }
    }

    void APU::clock()
    {
        // Los canales de pulso en la NES real se actualizan cada 2 ciclos de CPU.
        static bool pulseDivider = false;
        if (pulseDivider)
        {
            pulse1.clockTimer();
            pulse2.clockTimer();
        }
        pulseDivider = !pulseDivider;
        triangle.clockTimer();
        noise.clockTimer();
        dmc.clockTimer();

        cpuCycleCount++;

        const uint32_t CYCLES_PER_STEP = 7457;

        if(cpuCycleCount >= CYCLES_PER_STEP)
        {
            cpuCycleCount -= CYCLES_PER_STEP;

            if(!fiveStepMode)
            {
                switch(frameStep)
                {
                    case 0:
                        pulse1.clockEnvelope();
                        pulse2.clockEnvelope();
                        triangle.clockLinearCounter();
                        noise.clockEnvelope();
                        break;
                    case 1:
                        pulse1.clockEnvelope();
                        pulse2.clockEnvelope();
                        triangle.clockLinearCounter();
                        noise.clockEnvelope();

                        pulse1.clockLengthCounter();
                        pulse2.clockLengthCounter();
                        triangle.clockLengthCounter();
                        noise.clockLengthCounter();

                        pulse1.clockSweep();
                        pulse2.clockSweep();
                        break;
                    case 2:
                        pulse1.clockEnvelope();
                        pulse2.clockEnvelope();
                        triangle.clockLinearCounter();
                        noise.clockEnvelope();
                        break;
                    case 3:
                        pulse1.clockEnvelope();
                        pulse2.clockEnvelope();
                        triangle.clockLinearCounter();
                        noise.clockEnvelope();

                        pulse1.clockLengthCounter();
                        pulse2.clockLengthCounter();
                        triangle.clockLengthCounter();
                        noise.clockLengthCounter();

                        pulse1.clockSweep();
                        pulse2.clockSweep();

                        if(!irqInhibit)
                        {

                        }
                        break;
                }

                frameStep = (frameStep + 1)% 4;
            }
            else
            {
                switch(frameStep)
                {
                    case 0:
                    case 2:
                        pulse1.clockEnvelope();
                        pulse2.clockEnvelope();
                        triangle.clockLinearCounter();
                        noise.clockEnvelope();
                        break;
                    case 1:
                    case 3:
                        pulse1.clockEnvelope();
                        pulse2.clockEnvelope();
                        triangle.clockLinearCounter();
                        noise.clockEnvelope();

                        pulse1.clockLengthCounter();
                        pulse2.clockLengthCounter();
                        triangle.clockLengthCounter();
                        noise.clockLengthCounter();

                        pulse1.clockSweep();
                        pulse2.clockSweep();
                        break;
                    case 4:
                        break;
                }
                frameStep = (frameStep + 1) % 5;
            }
        }
    }

    double APU::getAudioSample()
    {
        uint8_t p1 = pulse1.output();
        uint8_t p2 = pulse2.output();
        uint8_t t = triangle.output();
        uint8_t n = noise.output();
        uint8_t d = dmc.output();

        static const double pulseTable[31] = {
            0.0f,
            0.00752f, 0.01493f, 0.02223f, 0.02943f, 0.03653f, 0.04353f, 0.05043f,
            0.05724f, 0.06395f, 0.07056f, 0.07708f, 0.08351f, 0.08985f, 0.09610f,
            0.10226f, 0.10834f, 0.11434f, 0.12025f, 0.12608f, 0.13184f, 0.13752f,
            0.14312f, 0.14865f, 0.15410f, 0.15948f, 0.16479f, 0.17003f, 0.17520f,
            0.18030f, 0.18534f
        };

        double pulseOut = 0.0f;
        if((p1 + p2) > 0)
        {
            pulseOut = 95.88f / (((double)8128 / static_cast<double> (p1 + p2)) + (double)(100));
        }

        double tndOut = 0.0f;
        double tndSum = (static_cast<double>(t) / (double)(8227)) +
                        (static_cast<double>(n) / (double)(12241)) +
                        (static_cast<double>(d) / (double)(22638));
        
        if(tndSum > (double)(0))
        {
            tndOut = (double)(159.79f) / (((double)1.0f / tndSum) + (double)(100.0f));
        }

        return pulseOut + tndOut;
    }

    double APU::getTestToneSample()
    {
        static double phase = 0.0;
        const double frequency = 440.0;
        const double sampleRate = 44100.0;

        float sample = static_cast<double> (0.2 * std::sin(2.0 * 3.1415926535 * phase));

        phase += frequency / sampleRate;
        if(phase >= 1.0) phase -= 1.0;

        return sample;
    }

    void APU::activateTestMode()
    {
        testMode = true;
    }

    void APU::deactivateTestMode()
    {
        testMode = false;
    }
}