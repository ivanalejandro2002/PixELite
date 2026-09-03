#include <iostream>
#include <NES/NES.h>

namespace NES
{
    NES::NES()
        :   ram(),
            prgRom(createTestProgram()),
            mainBus(),
            cpu()
    {
        cpu.connectBus(&mainBus);

        mainBus.addDevice(&ram);

        mainBus.addAddressMask({
            0x0000,
            0x1FFF,
            0x07FF,
            0x0000
        });

        mainBus.addDevice(&prgRom);

        reset();
    }

    void NES::reset()
    {
        masterClock = 0;
        cpu.reset();
    }

    void NES::clock()
    {
        if(masterClock % 3 == 0)
        {
            cpu.clock();
        }

        masterClock++;
    }

    void NES::mock()
    {
        cpu.mock();

    }

    std::vector<uint8_t> NES::createTestProgram()
    {
        std::vector<uint8_t> program(0x8000, 0x00);

        // $8000: LDA #$42
        program[0x0000] = 0xA9;
        program[0x0001] = 0x42;

        // Reset vector: $FFFC/$FFFD -> $8000
        program[0x7FFC] = 0x00;
        program[0x7FFD] = 0x80;

        return program;
    }
}