#include <iostream>
#include <NES/NES.h>

int main()
{
    NES::CPU::MOS6502 cpu;

    cpu.mock();

    return 0;
}