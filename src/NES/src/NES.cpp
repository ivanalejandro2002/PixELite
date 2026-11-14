#include <iostream>
#include <NES/NES.h>
#include <fstream>
#include <filesystem>

namespace NES
{
    NES::NES()
        :   ram(),
            mainBus(),
            cpu(),
            cartridge(), // <-- Solo el cartridge, ya no prgRom
            ppu(&cartridge, Mirroring::Horizontal), // <-- Puntuando al cartridge real para la CHR-RAM/ROM
            ppuRegisters(ppu),
            apu(mainBus),
            controller1()
    {
        cpu.connectBus(&mainBus);
        ppuRegisters.connectMainBus(&mainBus);

        mainBus.setDMCHandler({
            [this]() { return apu.getDMC().isDMARequested(); },
            [this]() { return apu.getDMC().getDMAAddress(); },
            [this](uint8_t sample) { apu.getDMC().deliverDMASample(sample); },
            [this]() { apu.clearFrameIRQ(); },
            [this]() { return cpu.getCurrentBusAddress(); },
            [this]() { return apu.getDMC().isAbortedDMARequested(); },
            [this]() { apu.getDMC().clearAbortedDMARequest(); }
        });

        mainBus.addDevice(&ram);
        mainBus.addDevice(&ppuRegisters);
        mainBus.addDevice(&apu);
        mainBus.addDevice(&controller1);

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

        mainBus.addDevice(&cartridge);

        reset();
    }

    void NES::reset()
    {
        masterClock = 0;
        mainBus.resetBus();
        ppu.reset();
        apu.reset();
        cartridge.resetMapper();
        cpu.reset();
    }

    void NES::clock()
    {
        ppu.clock();
        if(masterClock % 3 == 0)
        {
            apu.clock();
            controller1.clock(mainBus.totalCpuCycles);

            bool cpuWriting = cpu.isWriteCycle();

            // Si un DMA abortado de 1 ciclo cae en un ciclo de escritura de la CPU,
            // no se retrasa: simplemente no ocurre (0 ciclos):
            if(apu.getDMC().isAbortedDMARequested() && cpuWriting && !mainBus.isDMAActive())
            {
                apu.getDMC().clearAbortedDMARequest();
            }

            bool dmcPending = mainBus.isDMCRunning() ||
                              apu.getDMC().isDMARequested() ||
                              apu.getDMC().isAbortedDMARequested();

            bool canPauseCPU = !cpuWriting || mainBus.isDMAActive();

            if(dmcPending && canPauseCPU)
            {
                mainBus.clockDMCDMA();
            }
            else if(mainBus.isDMAActive() && !cpuWriting)
            {
                mainBus.clockDMA();
            }
            else
            {
                cpu.clock();
            }

            mainBus.totalCpuCycles++;
        }

        masterClock++;
    }

    bool NES::loadROM(const std::string &filepath)
    {
        if (cartridge.loadFromFile(filepath))
        {
            ppu.getCiram().setMirroring(cartridge.getMirroring());
            reset();

            std::cout << "[NES] Sistema inicializado con la ROM: " << filepath << "\n";
        }
        else
        {
            std::cerr << "[NES] Fallo crítico al intentar cargar la ROM.\n";
            return false;
        }
        return true;
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

    APU& NES::getAPU()
    {
        return apu;
    }

    Controller& NES::getController1()
    {
        return controller1;
    }

    PPU2C02& NES::getPPU()
    {
        return ppu;
    }

    void NES::memoryDebug()
    {
        std::error_code ec;
        std::filesystem::path rootPath = std::filesystem::current_path(ec);
        if (ec) {
            rootPath = std::filesystem::path(".");
        }

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

        file << "Cartridge data:\n";

        for(uint16_t i = 0x6000; i <= 0x07FFF; ++i)
        {
            if(!(i & (0x00FF)))
                file << "\nPage: " << i << ", " << i + 0x00FF << ":\n";

            if(!(i & (0x000F)))
                file << "\n";

            file << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                << static_cast<int>(mainBus.read(i)) << " "; 
        }

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

    void NES::cpuWrite(uint16_t address, uint8_t value)
    {
        mainBus.write(address, value);
    }

    uint8_t NES::cpuRead(uint16_t address)
    {
        return mainBus.read(address);
    }

    std::vector<uint8_t> NES::createTestProgram()
    {
        std::vector<uint8_t> program(0x8000, 0x00);

        // --- Inicialización ---
        // $8000: LDX #$00  (X se usará tanto para el VALOR como para la DIRECCIÓN)
        program[0x0000] = 0xA2; program[0x0001] = 0x00;

        // --- Bucle Principal (Loop) ---
        // $8002: TXA       (Transfiere el valor de X al Acumulador A)
        program[0x0002] = 0x8A;
        
        // $8003: STA $00,X (Guarda el valor de A en la dirección ZeroPage $00 + X)
        program[0x0003] = 0x95; program[0x0004] = 0x00;
        
        // $8005: INX       (Incrementa X. Al pasar de 0xFF, X vuelve a 0x00 y activa flag Z)
        program[0x0005] = 0xE8;
        
        // $8006: BNE $8002 (Branch if Not Equal. Si Z=0, salta 5 bytes hacia atrás a $8002)
        // El offset en complemento a dos para retroceder 5 bytes desde $8008 es 0xFB
        program[0x0006] = 0xD0; program[0x0007] = 0xFA;

        // --- Fin del Programa ---
        // $8008: KIL / JAM (Se ejecuta solo cuando X se desborda a 0x00 y Z se vuelve 1)
        program[0x0008] = 0xB2;

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


    bool NES::saveStateToFile(const std::string& filepath) const
    {
        std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            std::cerr << "[NES] Error al crear archivo savestate: " << filepath << "\n";
            return false;
        }

        // 1. Cabecera mágica (.pxsave) y Mapper ID para validar compatibilidad
        const char magic[4] = {'P', 'X', 'S', 'V'};
        out.write(magic, 4);
        uint32_t version = 1;
        uint16_t mapperId = cartridge.getMapperNumber();
        SaveStateStream::writeVal(out, version);
        SaveStateStream::writeVal(out, mapperId);

        // 2. Reloj Maestro y Registros de Subsistemas
        SaveStateStream::writeVal(out, masterClock);
        cpu.saveState(out);
        ppu.saveState(out);
        apu.saveState(out);
        mainBus.saveState(out);

        // 3. Bancos de Memoria (RAM 2KB, CIRAM, PRG-RAM y Mapper)
        ram.saveState(out);
        cartridge.saveState(out);

        out.close();
        std::cout << "[NES] Savestate guardado exitosamente en: " << filepath << "\n";
        return true;
    }

    bool NES::loadStateFromFile(const std::string& filepath)
    {
        std::ifstream in(filepath, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "[NES] No se pudo abrir el savestate: " << filepath << "\n";
            return false;
        }

        char magic[4] = {};
        in.read(magic, 4);
        if (std::memcmp(magic, "PXSV", 4) != 0) {
            std::cerr << "[NES] Archivo .pxsave inválido o corrupto.\n";
            return false;
        }

        uint32_t version = 0;
        uint16_t mapperId = 0;
        SaveStateStream::readVal(in, version);
        SaveStateStream::readVal(in, mapperId);

        if (mapperId != cartridge.getMapperNumber()) {
            std::cerr << "[NES] El savestate pertenece a otro Mapper (" << mapperId
                    << " vs " << cartridge.getMapperNumber() << ").\n";
            return false;
        }

        SaveStateStream::readVal(in, masterClock);
        cpu.loadState(in);
        ppu.loadState(in);
        apu.loadState(in);
        mainBus.loadState(in);

        ram.loadState(in);
        cartridge.loadState(in);

        in.close();
        std::cout << "[NES] Savestate cargado exitosamente desde: " << filepath << "\n";
        return true;
    }
}