#include <iostream>
#include <NES/NES.h>

namespace NES
{
    NES::NES()
    {
        cpu.connectBus(&mainBus);

        mainBus.addDevice(&ram);

        mainBus.addAddressMask({
            0x0000,
            0x1FFF,
            0x07FF,
            0x0000
        });

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
}