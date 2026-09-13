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

    std::vector<uint8_t> NES::createTestProgram()
    {
        std::vector<uint8_t> program(0x8000, 0x00);

        // ==========================================
        // 1. TEST LAX ($AF): Carga A y X con valor en memoria
        // Target $0210 = 0xC0
        // - LAX $0210 -> A = 0xC0, X = 0xC0
        // - Banderas -> N=1 (bit 7 activo), Z=0
        // ==========================================
        program[0x0000] = 0xA9; program[0x0001] = 0xC0; // $8000: LDA #$C0
        program[0x0002] = 0x8D; program[0x0003] = 0x10; program[0x0004] = 0x02; // $8002: STA $0210
        
        // Limpia A y X para comprobar que LAX los sobreescribe a ambos
        program[0x0005] = 0xA9; program[0x0006] = 0x00; // $8005: LDA #$00
        program[0x0007] = 0xAA;                         // $8007: TAX (X = 0x00)

        // $8008: LAX $0210 (Opcode 0xAF - LAX Absoluto)
        program[0x0008] = 0xAF; program[0x0009] = 0x10; program[0x000A] = 0x02;

        // Guarda A, X y Banderas del Test 1 (LAX)
        program[0x000B] = 0x8D; program[0x000C] = 0x00; program[0x000D] = 0x02; // $800B: STA $0200 (A = 0xC0)
        program[0x000E] = 0x8E; program[0x000F] = 0x01; program[0x0010] = 0x02; // $800E: STX $0201 (X = 0xC0)
        program[0x0011] = 0x08; // $8011: PHP
        program[0x0012] = 0x68; // $8012: PLA
        program[0x0013] = 0x8D; program[0x0014] = 0x02; program[0x0015] = 0x02; // $8013: STA $0202 (Banderas tras LAX)

        // ==========================================
        // 2. TEST SAX ($8F): Guarda (A AND X) en Memoria
        // A = 0x0F (0000 1111), X = 0x33 (0011 0011)
        // - A AND X = 0x03 (0000 0011)
        // - SAX $0211 -> Escribe 0x03 en la dirección $0211
        // ==========================================
        program[0x0016] = 0xA9; program[0x0017] = 0x0F; // $8016: LDA #$0F
        program[0x0018] = 0xA2; program[0x0019] = 0x33; // $8018: LDX #$33

        // $801A: SAX $0211 (Opcode 0x8F - SAX Absoluto)
        program[0x001A] = 0x8F; program[0x001B] = 0x11; program[0x001C] = 0x02;

        // Guarda las Banderas del Test 2 para confirmar que SAX no las altera
        program[0x001D] = 0x08; // $801D: PHP
        program[0x001E] = 0x68; // $801E: PLA
        program[0x001F] = 0x8D; program[0x0020] = 0x03; program[0x0021] = 0x02; // $801F: STA $0203 (Banderas tras SAX)

        // ==========================================
        // FIN DEL PROGRAMA
        // ==========================================
        program[0x0022] = 0x02; // $8022: KIL

        // Vectores de Reset ($FFFC/$FFFD)
        program[0x7FFC] = 0x00;
        program[0x7FFD] = 0x80;

        return program;

    }
}