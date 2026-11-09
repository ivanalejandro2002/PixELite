#include <NES/Controller/Controller.h>

namespace NES
{
    Controller::Controller()
    {
        buttonStates = 0;
        shiftRegister = 0;
        strobeState = 0;
    }

    bool Controller::handlesRead(uint16_t addr) const {return addr == 0x4016;}
    bool Controller::handlesWrite(uint16_t addr) const {return addr == 0x4016;}

    uint8_t Controller::read(uint16_t addr)
    {
        if(strobeState) shiftRegister = buttonStates;

        uint8_t response = shiftRegister & 0x01;
        shiftRegister >>= 1;

        return response;
    }

    void Controller::write(uint16_t addr, uint8_t data)
    {
        strobeState = (data & 0x01);
        if(strobeState) shiftRegister = buttonStates;
    }

    void Controller::setButtonState(Button button, bool pressed)
    {
        if(pressed)
        {
            buttonStates |= button;
        }
        else
        {
            buttonStates &= ~button;
        }
    }
}