#include <iostream>
#include <NES/NES.h>

int main()
{
    NES::NES nes;

    while(!nes.isCpuJammed())
    {
        nes.clock();
    }

    nes.memoryDebug();

    // nes.mock();

    return 0;
}