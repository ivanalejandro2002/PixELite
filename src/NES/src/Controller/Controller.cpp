#include <NES/Controller/Controller.h>

namespace NES
{
    Controller::Controller()
        : buttonStates(0), shiftRegister(0xFF), shiftRegister2(0xFF), strobeState(false)
    {}

    bool Controller::handlesRead(uint16_t addr) const
    {
        return addr == 0x4016 || addr == 0x4017;
    }

    bool Controller::handlesWrite(uint16_t addr) const
    {
        return addr == 0x4016;
    }

    uint8_t Controller::read(uint16_t addr)
    {
        if (addr == 0x4016)
        {
            if (strobeState)
            {
                shiftRegister = buttonStates;
                return shiftRegister & 0x01;
            }
            uint8_t response = shiftRegister & 0x01;
            shiftRegister = (shiftRegister >> 1) | 0x80;
            return response;
        }
        else if (addr == 0x4017)
        {
            if (strobeState)
            {
                shiftRegister2 = 0;
                return 0;
            }
            uint8_t response = shiftRegister2 & 0x01;
            shiftRegister2 = (shiftRegister2 >> 1) | 0x80;
            return response;
        }
        return 0;
    }

    void Controller::write(uint16_t addr, uint8_t value)
    {
        if (addr == 0x4016)
        {
            pendingStrobeValue = (value & 0x01) != 0;
            pendingStrobe = true;
        }
    }

    void Controller::clock(uint64_t totalCpuCycles)
    {
        // El latch OUT0 de $4016 solo se actualiza en la transición GET -> PUT (ciclo PUT, impar)
        if (pendingStrobe && (totalCpuCycles & 1) == 1)
        {
            strobeState = pendingStrobeValue;
            pendingStrobe = false;
            if (strobeState)
            {
                shiftRegister = buttonStates;
                shiftRegister2 = 0;
            }
        }
    }

    void Controller::setButtonState(Button button, bool pressed)
    {
        if (pressed)
            buttonStates |= button;
        else
            buttonStates &= ~button;

        if (strobeState)
            shiftRegister = buttonStates;
    }
}