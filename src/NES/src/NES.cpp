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
            cpu(),
            ppu(createTestCHR(), CIRAM::Mirroring::Horizontal),
            ppuRegisters(ppu)
    {
        cpu.connectBus(&mainBus);
        ppuRegisters.connectMainBus(&mainBus);

        mainBus.addDevice(&ram);
        mainBus.addDevice(&ppuRegisters);

        mainBus.addAddressMask({
            0x0000,
            0x1FFF,
            0x07FF,
            0x0000
        });

        mainBus.addAddressMask({
            0x2000,
            0x3FFF,
            0x0007,
            0x2000
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
        ppu.clock();
        if(!mainBus.isDMARunning())
        {
            if(masterClock % 3 == 0)
            {
                cpu.clock();
            }
        }
        else
        {
            mainBus.countDownDMA();
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

    const PPU2C02& NES::getPPU() const
    {
        return ppu;
    }

    PPU2C02& NES::getPPU()
    {
        return ppu;
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

        file << "CIRAM / NAMETABLES:\n";
        for (uint16_t addr = 0x2000; addr < 0x3000; addr += 16) {
            if((addr & 0x03FF) == 0)
            {
                file << "Nametable: " << (addr-0x2000)/0x0400 + 1 << ":\n";
            }
            file << "$"
                    << std::hex << std::uppercase
                    << std::setw(4) << std::setfill('0') << addr
                    << ": ";

            for (uint16_t i = 0; i < 16; i++) {
                file << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                    << static_cast<int>(ppu.readVRAM(addr + i)) << " ";
            }

            file << '\n';
        }

        file << "\n\n";

        file << "PALETTE RAM:\n";

        for(uint addr = 0x3F00; addr <= 0x3F0F; ++addr)
        {
            file << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                    << static_cast<int>(ppu.readVRAM(addr)) << " ";
        }
        file << "\n";

        for(uint addr = 0x3F10; addr <= 0x3F1F; ++addr)
        {
            file << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                    << static_cast<int>(ppu.readVRAM(addr)) << " ";
        }
        file << "\n\n";


        file << "PPU REGISTERS:\n";
        file << "$2000 -> " << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                << static_cast<int>(ppu.debugPPUCTRL()) << "\n";
        file << "$2001 -> " << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                << static_cast<int>(ppu.debugPPUMASK()) << "\n";
        file << "$2002 -> " << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                << static_cast<int>(ppu.debugPPUSTATUS()) << "\n";
        file << "$2003 -> " << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                << static_cast<int>(ppu.debugOAMADDR()) << "\n";
        file << "$2004 -> " << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                << static_cast<int>(ppu.debugOAMDATA()) << "\n";

        file << "\n\n";

        file << "PPU INTERNAL REGISTERS:\n";
        file << "v -> " << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                << static_cast<int>(ppu.debugV()) << "\n";
        file << "t -> " << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                << static_cast<int>(ppu.debugT()) << "\n";
        file << "x -> " << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                << static_cast<int>(ppu.debugX()) << "\n";
        file << "w -> " << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                << static_cast<int>(ppu.debugW()) << "\n";

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

        // --- Configuración de PPUCTRL ---
        // $8000: LDA #$80
        program[0x0000] = 0xA9; program[0x0001] = 0x80;

        // $8002: STA $2000 (PPUCTRL = 0x80)
        program[0x0002] = 0x8D; program[0x0003] = 0x00; program[0x0004] = 0x20;

        // --- Configuración de PPUMASK ---
        // $8005: LDA #$1E
        program[0x0005] = 0xA9; program[0x0006] = 0x1E;

        // $8007: STA $2001 (PPUMASK = 0x1E)
        program[0x0007] = 0x8D; program[0x0008] = 0x01; program[0x0009] = 0x20;

        // --- Configurar dirección VRAM: $2000 ---
        // $800A: LDA #$20
        program[0x000A] = 0xA9; program[0x000B] = 0x20;

        // $800C: STA $2006 (byte alto de PPUADDR)
        program[0x000C] = 0x8D; program[0x000D] = 0x06; program[0x000E] = 0x20;

        // $800F: LDA #$00
        program[0x000F] = 0xA9; program[0x0010] = 0x00;

        // $8011: STA $2006 (byte bajo de PPUADDR; dirección final = $2000)
        program[0x0011] = 0x8D; program[0x0012] = 0x06; program[0x0013] = 0x20;

        // --- Escribir datos en CIRAM mediante PPUDATA ---
        // $8014: LDA #$23
        program[0x0014] = 0xA9; program[0x0015] = 0x23;

        // $8016: STA $2007 (VRAM[$2000] = 0x23)
        program[0x0016] = 0x8D; program[0x0017] = 0x07; program[0x0018] = 0x20;

        // $8019: LDA #$45
        program[0x0019] = 0xA9; program[0x001A] = 0x45;

        // $801B: STA $2007 (VRAM[$2001] = 0x45)
        program[0x001B] = 0x8D; program[0x001C] = 0x07; program[0x001D] = 0x20;

        // $801E: LDA #$67
        program[0x001E] = 0xA9; program[0x001F] = 0x67;

        // $8020: STA $2007 (VRAM[$2002] = 0x67)
        program[0x0020] = 0x8D; program[0x0021] = 0x07; program[0x0022] = 0x20;

        // $8023: LDA #$89
        program[0x0023] = 0xA9; program[0x0024] = 0x89;

        // $8025: STA $2007 (VRAM[$2003] = 0x89)
        program[0x0025] = 0x8D; program[0x0026] = 0x07; program[0x0027] = 0x20;

        // --- Configurar dirección PaletteRAM: $3F00 ---
        // $8028: LDA #$3F
        program[0x0028] = 0xA9; program[0x0029] = 0x3F;

        // $802A: STA $2006 (byte alto de PPUADDR)
        program[0x002A] = 0x8D; program[0x002B] = 0x06; program[0x002C] = 0x20;

        // $802D: LDA #$00
        program[0x002D] = 0xA9; program[0x002E] = 0x00;

        // $802F: STA $2006 (byte bajo de PPUADDR; dirección final = $3F00)
        program[0x002F] = 0x8D; program[0x0030] = 0x06; program[0x0031] = 0x20;

        // $8032: LDA #$2A
        program[0x0032] = 0xA9; program[0x0033] = 0x2A;

        // $8034: STA $2007 (PaletteRAM[$3F00] = 0x2A)
        program[0x0034] = 0x8D; program[0x0035] = 0x07; program[0x0036] = 0x20;

        // --- Fin del Programa ---
        // $8037: KIL / JAM (opcode asignado por tu implementación)
        program[0x0037] = 0xB2;

        // Vectores de Reset ($FFFC/$FFFD apuntando a $8000)
        program[0x7FFC] = 0x00;
        program[0x7FFD] = 0x80;

        return program;

    }

    std::vector<uint8_t> NES::createTestCHR()
    {
        // CHR de 8 KiB, inicializado con tiles vacíos.
        std::vector<uint8_t> chr(0x2000, 0x00);

        // =========================================================
        // TILE 0: Cuadrícula alternada.
        // Usa los valores de píxel 0 y 3.
        // Dirección CHR: $0000 - $000F
        // =========================================================

        chr[0x00] = 0xAA; // Plano 0, fila 0: 10101010
        chr[0x01] = 0x55; // Plano 0, fila 1: 01010101
        chr[0x02] = 0xAA;
        chr[0x03] = 0x55;
        chr[0x04] = 0xAA;
        chr[0x05] = 0x55;
        chr[0x06] = 0xAA;
        chr[0x07] = 0x55;

        chr[0x08] = 0xAA; // Plano 1
        chr[0x09] = 0x55;
        chr[0x0A] = 0xAA;
        chr[0x0B] = 0x55;
        chr[0x0C] = 0xAA;
        chr[0x0D] = 0x55;
        chr[0x0E] = 0xAA;
        chr[0x0F] = 0x55;

        // =========================================================
        // TILE 1: Diagonal.
        // Usa el valor de píxel 1.
        // Dirección CHR: $0010 - $001F
        // =========================================================

        chr[0x10] = 0x80; // Plano 0, fila 0: 10000000
        chr[0x11] = 0x40; // Plano 0, fila 1: 01000000
        chr[0x12] = 0x20; // Plano 0, fila 2: 00100000
        chr[0x13] = 0x10; // Plano 0, fila 3: 00010000
        chr[0x14] = 0x08; // Plano 0, fila 4: 00001000
        chr[0x15] = 0x04; // Plano 0, fila 5: 00000100
        chr[0x16] = 0x02; // Plano 0, fila 6: 00000010
        chr[0x17] = 0x01; // Plano 0, fila 7: 00000001

        // El plano 1 permanece en cero:
        // chr[0x18] ... chr[0x1F] = 0x00.

        // =========================================================
        // TILE 2: Marco.
        // Usa el valor de píxel 2.
        // Dirección CHR: $0020 - $002F
        // =========================================================

        chr[0x20] = 0x00; // Plano 0: todas las filas vacías
        chr[0x21] = 0x00;
        chr[0x22] = 0x00;
        chr[0x23] = 0x00;
        chr[0x24] = 0x00;
        chr[0x25] = 0x00;
        chr[0x26] = 0x00;
        chr[0x27] = 0x00;

        chr[0x28] = 0xFF; // Plano 1: borde superior
        chr[0x29] = 0x81; // Borde izquierdo y derecho
        chr[0x2A] = 0x81;
        chr[0x2B] = 0x81;
        chr[0x2C] = 0x81;
        chr[0x2D] = 0x81;
        chr[0x2E] = 0x81;
        chr[0x2F] = 0xFF; // Borde inferior


        chr[0x30] = 0x41;
        chr[0x31] = 0xC2;
        chr[0x32] = 0x44;
        chr[0x33] = 0x48;
        chr[0x34] = 0x10;
        chr[0x35] = 0x20;
        chr[0x36] = 0x40;
        chr[0x37] = 0x80;

        chr[0x38] = 0x01;
        chr[0x39] = 0x02;
        chr[0x3A] = 0x04;
        chr[0x3B] = 0x08;
        chr[0x3C] = 0x16;
        chr[0x3D] = 0x21;
        chr[0x3E] = 0x42;
        chr[0x3F] = 0x87;

        chr[0x1000] = 0x41;
        chr[0x1001] = 0xC2;
        chr[0x1002] = 0x44;
        chr[0x1003] = 0x48;
        chr[0x1004] = 0x10;
        chr[0x1005] = 0x20;
        chr[0x1006] = 0x40;
        chr[0x1007] = 0x80;

        chr[0x1008] = 0x01;
        chr[0x1009] = 0x02;
        chr[0x100A] = 0x04;
        chr[0x100B] = 0x08;
        chr[0x100C] = 0x16;
        chr[0x100D] = 0x21;
        chr[0x100E] = 0x42;
        chr[0x100F] = 0x87;

        return chr;
    }
}