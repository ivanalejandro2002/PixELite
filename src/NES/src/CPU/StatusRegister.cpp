#include <NES/CPU/StatusRegister.h>

namespace NES::CPU
{
    StatusRegister::StatusRegister()
    {
        flags = 0;
    }

    void StatusRegister::setFlag(FlagID flag, bool value)
    {
        if(value) flags |= (1 << flag);
        else flags &= ~(1<< flag);
    }

    bool StatusRegister::getFlag(FlagID flag) const
    {
        return (flags >> flag) & 1;
    }
    
    uint8_t StatusRegister::getByte() const
    {
        return flags;
    }

    void StatusRegister::setByte(uint8_t value)
    {
        flags = value;
    }

    void StatusRegister::applyMask(uint8_t values, uint8_t mask)
    {
        flags &= ~mask;
        flags |= (values & mask);
    }
}