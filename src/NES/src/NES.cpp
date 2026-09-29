#include <iostream>
#include <NES/NES.h>
#include <fstream>
#include <filesystem>

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

    bool NES::isCpuJammed()
    {
        return cpu.isJammed;
    }

    void NES::memoryDebug()
    {
        std::filesystem::path rootPath(PROJECT_ROOT_DIR);

        std::filesystem::path realPath = rootPath / "debugs" / "ROMTest.out";

        std::filesystem::create_directories(realPath.parent_path()); 
        
        std::ofstream file(realPath);

        file << "RAM:\n";
        for(uint16_t i = 0; i <= 0x07FF; ++i)
        {
            if(!(i & (0x00FF)))
                file << "\nPage: " << (short)(i >> 8) << ":\n";

            if(!(i & (0x000F)))
                file << "\n";

            file << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                << static_cast<int>(mainBus.read(i)) << " "; 
        }

        file << "\n\n";

        file.close();
    }

    void NES::debugNMI()
    {
        mainBus.setNMILine(true);
        mainBus.setNMILine(false);
    }

    void NES::debugIRQ()
    {
        mainBus.setIRQLine(false);
    }

    void NES::debugHighIRQ()
    {
        mainBus.setIRQLine(true);
    }

    std::vector<uint8_t> NES::createTestProgram()
    {
        std::vector<uint8_t> program(0x8000, 0x00);

        program[0x0000] = 0x58; // CLI (2 ciclos) -> Limpia I = 0. En su polling detecta IRQ
        program[0x0001] = 0xEA; // NOP (Se ejecuta tras volver de RTI)
        program[0x0002] = 0x02; // KIL / JAM (Detiene la CPU)

        // 2. Rutina de Servicio de IRQ en $8030 (Índice 0x0030)
        // NOTA: Tu código simulador/test debe simular que la IRQ se apaga aquí
        program[0x0030] = 0xA9; // LDA #$88
        program[0x0031] = 0x88;
        program[0x0032] = 0x85; // STA $01 -> Guarda A ($88) en RAM[0x0001]
        program[0x0033] = 0x01;
        program[0x0034] = 0x40; // RTI

        // 3. Vectores de Interrupción
        // Vector NMI ($FFFA/$FFFB) - No se usa en esta prueba
        program[0x7FFA] = 0x00; 
        program[0x7FFB] = 0x00; 

        // Vector RESET -> $8000 (Índice 0x7FFC / 0x7FFD)
        program[0x7FFC] = 0x00; // Low
        program[0x7FFD] = 0x80; // High

        // Vector IRQ / BRK -> $8030 (Índice 0x7FFE / 0x7FFF)
        program[0x7FFE] = 0x30; // Low
        program[0x7FFF] = 0x80; // High

        return program;

    }
}