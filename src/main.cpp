#include <iostream>
#include <NES/NES.h>

int main()
{
    NES::NES nes;

    nes.debugIRQ();

    int contador = 0;

    while(!nes.isCpuJammed())
    {
        if(contador == 25)nes.debugHighIRQ();
        nes.clock();
        ++contador;
    }

    nes.memoryDebug();

    // nes.mock();

    return 0;
}