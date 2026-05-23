#include <iostream>
#include <NES/CPU/ALU.h>
#include <NES/CPU/ALUResult.h>

namespace NES::CPU
{

    ALUResult::ALUResult(uint8_t value, uint8_t mode, uint8_t changedbits){
        result = value;
        statusBits = mode;
        statusChangedBits = changedbits;
    }

    ALUResult ALU::add(uint8_t a, uint8_t b, bool carryIn)
    {
        uint16_t sum = (uint16_t)a + (uint16_t)b + (uint16_t)carryIn;
        uint8_t result = sum & 0xFF;

        uint8_t flags = 0;
        if(sum & (0x100))
            flags |= ALUResult::FlagMask::C;
        if(!result)
            flags |= ALUResult::FlagMask::Z;
        if((~(a ^ b) & (a ^ result) & 0x80))
            flags |= ALUResult::FlagMask::V;
        if(result & (0x80))
            flags |= ALUResult::FlagMask::N;


        return
        {
            result,
            flags,
            ALUResult::FlagMask::C |
            ALUResult::FlagMask::Z |
            ALUResult::FlagMask::V |
            ALUResult::FlagMask::N
        };
    }

    ALUResult ALU::substract(uint8_t a, uint8_t b, bool carryIn)
    {
        uint8_t bInv = ~b;
        uint16_t diff = (uint16_t)a + (uint16_t)(bInv) + (uint16_t)carryIn;
        uint8_t result = diff & 0xFF;

        uint8_t flags = 0;
        if(diff & (0x100))
            flags |= ALUResult::FlagMask::C;
        if(!result)
            flags |= ALUResult::FlagMask::Z;
        if(((a ^ b) & (a ^ result) & 0x80))
            flags |= ALUResult::FlagMask::V;
        if(result & (0x80))
            flags |= ALUResult::FlagMask::N;


        return
        {
            result,
            flags,
            ALUResult::FlagMask::C |
            ALUResult::FlagMask::Z |
            ALUResult::FlagMask::V |
            ALUResult::FlagMask::N
        };
    }

    ALUResult ALU::andOp(uint8_t a, uint8_t b)
    {
        uint8_t result = a & b;
        
        uint8_t flags = 0;
        if(!result)
            flags |= ALUResult::FlagMask::Z;
        if(result & (0x80))
            flags |= ALUResult::FlagMask::N;


        return
        {
            result,
            flags,
            ALUResult::FlagMask::Z |
            ALUResult::FlagMask::N
        };
    }

    ALUResult ALU::orOp(uint8_t a, uint8_t b)
    {
        uint8_t result = a | b;
        
        uint8_t flags = 0;
        if(!result)
            flags |= ALUResult::FlagMask::Z;
        if(result & (0x80))
            flags |= ALUResult::FlagMask::N;


        return
        {
            result,
            flags,
            ALUResult::FlagMask::Z |
            ALUResult::FlagMask::N
        };
    }

    ALUResult ALU::xorOp(uint8_t a, uint8_t b)
    {
        uint8_t result = a ^ b;
        
        uint8_t flags = 0;
        if(!result)
            flags |= ALUResult::FlagMask::Z;
        if(result & (0x80))
            flags |= ALUResult::FlagMask::N;


        return
        {
            result,
            flags,
            ALUResult::FlagMask::Z |
            ALUResult::FlagMask::N
        };
    }

    ALUResult ALU::shiftLeft(uint8_t value)
    {
        uint8_t result = value << 1;

        uint8_t flags = 0;
        if(value & (0x80))
            flags |= ALUResult::FlagMask::C;
        if(!result)
            flags |= ALUResult::FlagMask::Z;
        if(result & (0x80))
            flags |= ALUResult::FlagMask::N;


        return
        {
            result,
            flags,
            ALUResult::FlagMask::C |
            ALUResult::FlagMask::Z |
            ALUResult::FlagMask::N
        };
    }

    ALUResult ALU::shiftRight(uint8_t value)
    {
        uint8_t result = value >> 1;

        uint8_t flags = 0;
        if(value & (0x01))
            flags |= ALUResult::FlagMask::C;
        if(!result)
            flags |= ALUResult::FlagMask::Z;
        if(result & (0x80))
            flags |= ALUResult::FlagMask::N;


        return
        {
            result,
            flags,
            ALUResult::FlagMask::C |
            ALUResult::FlagMask::Z |
            ALUResult::FlagMask::N
        };
    }

    ALUResult ALU::rotateLeft(uint8_t value, bool carryIn)
    {
        uint8_t result = (value << 1) | ((uint8_t)carryIn);

        uint8_t flags = 0;
        if(value & (0x80))
            flags |= ALUResult::FlagMask::C;
        if(!result)
            flags |= ALUResult::FlagMask::Z;
        if(result & (0x80))
            flags |= ALUResult::FlagMask::N;


        return
        {
            result,
            flags,
            ALUResult::FlagMask::C |
            ALUResult::FlagMask::Z |
            ALUResult::FlagMask::N
        };
    }

    ALUResult ALU::rotateRight(uint8_t value, bool carryIn)
    {
        uint8_t result = (value >> 1) | ((uint8_t)carryIn << 7);

        uint8_t flags = 0;
        if(value & (0x01))
            flags |= ALUResult::FlagMask::C;
        if(!result)
            flags |= ALUResult::FlagMask::Z;
        if(result & (0x80))
            flags |= ALUResult::FlagMask::N;


        return
        {
            result,
            flags,
            ALUResult::FlagMask::C |
            ALUResult::FlagMask::Z |
            ALUResult::FlagMask::N
        };
    }

    ALUResult ALU::increment(uint8_t value)
    {
        uint8_t result = (value == 0xFF)?0x00:value+1;

        uint8_t flags = 0;
        if(!result)
            flags |= ALUResult::FlagMask::Z;
        if(result & (0x80))
            flags |= ALUResult::FlagMask::N;


        return
        {
            result,
            flags,
            ALUResult::FlagMask::Z |
            ALUResult::FlagMask::N
        };
    }

    ALUResult ALU::decrement(uint8_t value)
    {
        uint8_t result = (value == 0x00)?0xFF:value-1;

        uint8_t flags = 0;
        if(!result)
            flags |= ALUResult::FlagMask::Z;
        if(result & (0x80))
            flags |= ALUResult::FlagMask::N;


        return
        {
            result,
            flags,
            ALUResult::FlagMask::Z |
            ALUResult::FlagMask::N
        };
    }

    ALUResult ALU::compare(uint8_t a, uint8_t b)
    {
        uint16_t sum = (uint16_t)a + (uint16_t)(~b) + 1;
        uint8_t result = sum & 0xFF;

        uint8_t flags = 0;
        if(a >= b)
            flags |= ALUResult::FlagMask::C;
        if(!result)
            flags |= ALUResult::FlagMask::Z;
        if(result & (0x80))
            flags |= ALUResult::FlagMask::N;


        return
        {
            result,
            flags,
            ALUResult::FlagMask::C |
            ALUResult::FlagMask::Z |
            ALUResult::FlagMask::N
        };
    }
}