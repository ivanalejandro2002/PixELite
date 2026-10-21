
#include <iostream>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include <NES/NES.h>
#include <Graphics/Renderer.h>
#include <cassert>

void fillNametable(NES::PPU2C02& ppu, uint16_t base, uint8_t tile)
{
    for (int y = 0; y < 30; ++y)
    {
        for (int x = 0; x < 32; ++x)
        {
            const uint16_t addr =
                base + y * 32 + x;

            ppu.writeVRAM(addr, tile);
        }
    }
}

void testScanLineTiming(NES::PPU2C02& ppu)
{
    ppu.reset();

    assert(ppu.debugScanline() == -1);
    assert(ppu.debugCycle() == 0);

    // Avanzar un ciclo.
    ppu.clock();

    assert(ppu.debugScanline() == -1);
    assert(ppu.debugCycle() == 1);

    // Completar los 340 ciclos restantes.
    for (int i = 0; i < 340; ++i)
        ppu.clock();

    // Debe comenzar la scanline visible 0.
    assert(ppu.debugScanline() == 0);
    assert(ppu.debugCycle() == 0);

    std::cout << "Prueba de scanline: OK\n";
}

void testFrameTiming(NES::PPU2C02& ppu)
{
    ppu.reset();

    constexpr int cyclesPerScanline = 341;
    constexpr int scanlinesPerFrame = 262;

    constexpr int cyclesPerFrame =
        cyclesPerScanline * scanlinesPerFrame;

    for (int i = 0; i < cyclesPerFrame; ++i)
        ppu.clock();

    assert(ppu.debugScanline() == -1);
    assert(ppu.debugCycle() == 0);

    std::cout << "Prueba de cuadro: OK\n";
}

void testVBlankTiming(NES::PPU2C02& ppu)
{
    ppu.reset();

    // Avanzar hasta el comienzo de la scanline 241.
    while (ppu.debugScanline() != 241 ||
           ppu.debugCycle() != 0)
    {
        ppu.clock();
    }

    // Antes del ciclo 1, VBlank debe seguir desactivado.
    assert((ppu.debugPPUSTATUS() & 0x80) == 0);

    ppu.clock();

    assert(ppu.debugScanline() == 241);
    assert(ppu.debugCycle() == 1);
    assert((ppu.debugPPUSTATUS() & 0x80) != 0);

    // Avanzar hasta el pre-render, ciclo 0.
    while (ppu.debugScanline() != -1 ||
           ppu.debugCycle() != 0)
    {
        ppu.clock();
    }

    ppu.clock();

    assert(ppu.debugScanline() == -1);
    assert(ppu.debugCycle() == 1);
    assert((ppu.debugPPUSTATUS() & 0x80) == 0);

    std::cout << "Prueba de VBlank: OK\n";
}

void testPPUSTATUS(NES::PPU2C02& ppu)
{
    ppu.reset();

    // Avanzar hasta el inicio de VBlank.
    while (ppu.debugScanline() != 241 ||
           ppu.debugCycle() != 0)
    {
        ppu.clock();
    }

    // Entrar al ciclo 1: VBlank se activa.
    ppu.clock();

    assert((ppu.debugPPUSTATUS() & 0x80) != 0);

    const uint8_t value = ppu.readPPUSTATUS();

    // La lectura debe devolver el estado anterior.
    assert((value & 0x80) != 0);

    // Después de la lectura, VBlank debe estar limpio.
    assert((ppu.debugPPUSTATUS() & 0x80) == 0);

    // Leer PPUSTATUS también reinicia el write toggle.
    assert(ppu.debugW() == false);

    std::cout << "Prueba de lectura de PPUSTATUS: OK\n";
}

void testPPUSCROLL(NES::PPU2C02& ppu)
{
    ppu.reset();

    // Primera escritura: scroll horizontal.
    ppu.writePPUSCROLL(0x2D);

    assert(ppu.debugX() == 0x2D % 8); // fineX = 5
    assert((ppu.debugT() & 0x001F) == (0x2D >> 3)); // coarseX = 5
    assert(ppu.debugW() == true);

    // Segunda escritura: scroll vertical.
    ppu.writePPUSCROLL(0xB6);

    assert(((ppu.debugT() >> 12) & 0x07) == (0xB6 & 0x07));
    assert(((ppu.debugT() >> 5) & 0x1F) == (0xB6 >> 3));
    assert(ppu.debugW() == false);

    std::cout << "Prueba de PPUSCROLL: OK\n";
}

void testPPUADDR(NES::PPU2C02& ppu)
{
    ppu.reset();

    // Primer byte: parte alta de la dirección.
    ppu.writePPUADDR(0x3F);

    assert(((ppu.debugT() >> 8) & 0x3F) == 0x3F);
    assert(ppu.debugW() == true);

    // Segundo byte: parte baja de la dirección.
    ppu.writePPUADDR(0x2A);

    const uint16_t expected = 0x3F2A;

    assert(ppu.debugT() == expected);
    assert(ppu.debugV() == expected);
    assert(ppu.debugW() == false);

    std::cout << "Prueba de PPUADDR: OK\n";
}

void testWriteToggleReset(NES::PPU2C02& ppu)
{
    ppu.reset();

    // Iniciar una secuencia de PPUSCROLL.
    ppu.writePPUSCROLL(0x2D);
    assert(ppu.debugW() == true);

    // Leer PPUSTATUS debe reiniciar el toggle.
    ppu.readPPUSTATUS();
    assert(ppu.debugW() == false);

    // Esta escritura debe comportarse como la primera.
    ppu.writePPUSCROLL(0x16);

    assert(ppu.debugW() == true);
    assert(ppu.debugX() == (0x16 & 0x07));
    assert((ppu.debugT() & 0x001F) == (0x16 >> 3));

    std::cout << "Prueba de reinicio del toggle: OK\n";
}

void testPPUDATAIncrement(NES::PPU2C02& ppu)
{
    ppu.reset();

    // PPUCTRL bit 2 = 0: incremento de 1.
    ppu.writePPUCTRL(0x00);
    ppu.writePPUADDR(0x21);
    ppu.writePPUADDR(0x00);

    assert(ppu.debugV() == 0x2100);

    ppu.writePPUDATA(0xA5);
    assert(ppu.debugV() == 0x2101);

    ppu.writePPUDATA(0x5A);
    assert(ppu.debugV() == 0x2102);

    // Verificar los datos directamente en VRAM.
    assert(ppu.readVRAM(0x2100) == 0xA5);
    assert(ppu.readVRAM(0x2101) == 0x5A);

    std::cout << "PPUDATA incremento de 1: OK\n";

    // PPUCTRL bit 2 = 1: incremento de 32.
    ppu.writePPUCTRL(0x04);
    ppu.writePPUADDR(0x22);
    ppu.writePPUADDR(0x00);

    assert(ppu.debugV() == 0x2200);

    ppu.writePPUDATA(0xC3);
    assert(ppu.debugV() == 0x2220);

    ppu.writePPUDATA(0x3C);
    assert(ppu.debugV() == 0x2240);

    assert(ppu.readVRAM(0x2200) == 0xC3);
    assert(ppu.readVRAM(0x2220) == 0x3C);

    std::cout << "PPUDATA incremento de 32: OK\n";
}

void testPPUDATAReadBuffer(NES::PPU2C02& ppu)
{
    ppu.reset();

    // Preparar dos bytes consecutivos en la nametable.
    ppu.writeVRAM(0x2100, 0xA5);
    ppu.writeVRAM(0x2101, 0x5A);

    // Establecer la dirección inicial.
    ppu.writePPUADDR(0x21);
    ppu.writePPUADDR(0x00);

    // La primera lectura devuelve el contenido previo del búfer.
    uint8_t first = ppu.readPPUDATA();
    assert(first == 0x00);

    // La segunda lectura devuelve el byte de 0x2100.
    uint8_t second = ppu.readPPUDATA();
    assert(second == 0xA5);

    // La tercera lectura devuelve el byte de 0x2101.
    uint8_t third = ppu.readPPUDATA();
    assert(third == 0x5A);

    std::cout << "PPUDATA búfer de lectura: OK\n";
}

void testPPUDATAPaletteRead(NES::PPU2C02& ppu)
{
    ppu.reset();

    ppu.writeVRAM(0x3F00, 0x2A);
    ppu.writeVRAM(0x3F01, 0x15);

    ppu.writePPUADDR(0x3F);
    ppu.writePPUADDR(0x00);

    // Las lecturas de paleta son inmediatas.
    assert(ppu.readPPUDATA() == 0x2A);
    assert(ppu.debugV() == 0x3F01);

    assert(ppu.readPPUDATA() == 0x15);
    assert(ppu.debugV() == 0x3F02);

    std::cout << "PPUDATA lectura de paleta: OK\n";
}

void testPaletteMirroring(NES::PPU2C02& ppu)
{
    ppu.reset();

    // Escribir en las entradas universales de fondo.
    ppu.writeVRAM(0x3F00, 0x2A);
    ppu.writeVRAM(0x3F04, 0x15);
    ppu.writeVRAM(0x3F08, 0x09);
    ppu.writeVRAM(0x3F0C, 0x30);

    // Leer mediante sus direcciones espejo.
    assert(ppu.readVRAM(0x3F10) == 0x2A);
    assert(ppu.readVRAM(0x3F14) == 0x15);
    assert(ppu.readVRAM(0x3F18) == 0x09);
    assert(ppu.readVRAM(0x3F1C) == 0x30);

    // También debe funcionar en el sentido inverso.
    ppu.writeVRAM(0x3F10, 0x12);
    assert(ppu.readVRAM(0x3F00) == 0x12);

    std::cout << "Espejos de Palette RAM: OK\n";
}

using NES::CIRAM;

void testVerticalMirroring()
{
    CIRAM ciram(CIRAM::Mirroring::Vertical);

    ciram.write(0x2000, 0x11);
    ciram.write(0x2400, 0x22);

    // NT0 y NT2 comparten memoria física.
    assert(ciram.read(0x2800) == 0x11);

    // NT1 y NT3 comparten memoria física.
    assert(ciram.read(0x2C00) == 0x22);

    std::cout << "CIRAM vertical mirroring: OK\n";
}

void testHorizontalMirroring()
{
    CIRAM ciram(CIRAM::Mirroring::Horizontal);

    ciram.write(0x2000, 0x11);
    ciram.write(0x2800, 0x22);

    // NT0 y NT1 comparten memoria física.
    assert(ciram.read(0x2400) == 0x11);

    // NT2 y NT3 comparten memoria física.
    assert(ciram.read(0x2C00) == 0x22);

    std::cout << "CIRAM horizontal mirroring: OK\n";
}

void testSingleScreenLower()
{
    CIRAM ciram(CIRAM::Mirroring::SingleScreenLower);

    ciram.write(0x2000, 0xA5);

    assert(ciram.read(0x2400) == 0xA5);
    assert(ciram.read(0x2800) == 0xA5);
    assert(ciram.read(0x2C00) == 0xA5);

    std::cout << "CIRAM single-screen lower: OK\n";
}

void testSingleScreenUpper()
{
    CIRAM ciram(CIRAM::Mirroring::SingleScreenUpper);

    ciram.write(0x2400, 0x5A);

    assert(ciram.read(0x2000) == 0x5A);
    assert(ciram.read(0x2800) == 0x5A);
    assert(ciram.read(0x2C00) == 0x5A);

    std::cout << "CIRAM single-screen upper: OK\n";
}

void testCIRAMAddressMirroring3000()
{
    CIRAM ciram(CIRAM::Mirroring::Vertical);

    ciram.write(0x2000, 0x31);
    ciram.write(0x2400, 0x42);

    // El rango 0x3000–0x3EFF refleja 0x2000–0x2EFF.
    assert(ciram.read(0x3000) == 0x31);
    assert(ciram.read(0x3400) == 0x42);

    std::cout << "CIRAM address mirroring 0x3000: OK\n";
}


using NES::PPU2C02;

std::array<uint8_t, 4> getPixel(
    const NES::Graphics::FrameBuffer& framebuffer,
    int x,
    int y
)
{
    const auto& pixels = framebuffer.getPixels();
    const size_t offset =
        (static_cast<size_t>(y) * NES::Graphics::FrameBuffer::Width + x)
        * NES::Graphics::FrameBuffer::BytesPerPixel;

    return {
        pixels[offset],
        pixels[offset + 1],
        pixels[offset + 2],
        pixels[offset + 3]
    };
}

void testBackgroundPixelExtraction()
{
    const auto chr = NES::NES::createTestCHR();

    PPU2C02 ppu(chr, NES::CIRAM::Mirroring::Horizontal);

    ppu.reset();

    // Seleccionar el tile 1 en la primera posición.
    ppu.writeVRAM(0x2000, 0x01);

    // Paleta de fondo.
    ppu.writeVRAM(0x3F00, 0x0F);
    ppu.writeVRAM(0x3F01, 0x01);

    // Tabla de patrones en $0000.
    ppu.writePPUCTRL(0x00);

    // Mostrar fondo y los primeros ocho píxeles.
    ppu.writePPUMASK(0x0A);

    // Avanzar por el pre-render y dibujar la primera scanline.
    for (int i = 0; i < 341 + 256; ++i)
        ppu.clock();

    const auto first = getPixel(ppu.getFramebuffer(), 0, 0);
    const auto second = getPixel(ppu.getFramebuffer(), 1, 0);

    // El primer píxel usa la entrada 1 de la paleta.
    // Los siguientes usan el color universal.
    assert(first != second);

    std::cout << "Extracción de píxeles del fondo: OK\n";
}

void testBackgroundShifterPipeline()
{
    const auto chr = NES::NES::createTestCHR();

    PPU2C02 ppu(chr, NES::CIRAM::Mirroring::Horizontal);

    ppu.reset();

    ppu.writeVRAM(0x2000, 0x01);

    ppu.writeVRAM(0x3F00, 0x0F);
    ppu.writeVRAM(0x3F01, 0x01);

    ppu.writePPUCTRL(0x00);
    ppu.writePPUMASK(0x0A);

    // Generar el primer scanline visible.
    for (int i = 0; i < 341 + 256; ++i)
        ppu.clock();

    const auto p0 = getPixel(ppu.getFramebuffer(), 0, 0);
    const auto p1 = getPixel(ppu.getFramebuffer(), 1, 0);
    const auto p7 = getPixel(ppu.getFramebuffer(), 7, 0);

    assert(p0 != p1);
    assert(p1 == p7);

    std::cout << "Pipeline de desplazamiento del fondo: OK\n";
}

void testPPUCTRLNametableSelection(NES::PPU2C02& ppu)
{
    ppu.reset();

    ppu.writePPUCTRL(0x03);

    assert((ppu.debugT() & 0x0C00) == 0x0C00);

    ppu.writePPUCTRL(0x01);

    assert((ppu.debugT() & 0x0C00) == 0x0400);

    ppu.writePPUCTRL(0x02);

    assert((ppu.debugT() & 0x0C00) == 0x0800);

    ppu.writePPUCTRL(0x00);

    assert((ppu.debugT() & 0x0C00) == 0x0000);

    std::cout << "Prueba de selección de nametable en PPUCTRL: OK\n";
}


int main()
{
    // Inicializar SDL3.
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Error al inicializar SDL3: "
                  << SDL_GetError() << '\n';
        return 1;
    }

    // Solicitar un contexto OpenGL 3.3 Core.
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_MAJOR_VERSION,
        3
    );

    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_MINOR_VERSION,
        3
    );

    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE
    );

    SDL_GL_SetAttribute(
        SDL_GL_DOUBLEBUFFER,
        1
    );

    // Crear ventana.
    SDL_Window* window = SDL_CreateWindow(
        "PixELite - NES",
        1024,
        960,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if(window == nullptr)
    {
        std::cerr << "Ventana no creada correctamente: "
                  << SDL_GetError() << '\n';

        SDL_Quit();
        return 1;
    }

    // Crear contexto OpenGL.
    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    if(glContext == nullptr)
    {
        std::cerr << "Error al crear contexto OpenGL: "
                  << SDL_GetError() << '\n';

        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Hacer actual el contexto.
    if(!SDL_GL_MakeCurrent(window, glContext))
    {
        std::cerr << "Error al activar contexto OpenGL: "
                  << SDL_GetError() << '\n';

        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Inicializar GLAD2.
    if(!gladLoadGL(
        reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)
    ))
    {
        std::cerr << "Error al inicializar GLAD\n";

        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    NES::NES nes;

    TT::Graphics::Renderer renderer;

    if(!renderer.initialize(window))
    {
        std::cerr << "Error al inicializar Renderer\n";

        renderer.destroy();
        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // nes.getPPU().writeRegister(0x2005, 10);
    // nes.getPPU().writeRegister(0x2005, 0);

    testScanLineTiming(nes.getPPU());
    testFrameTiming(nes.getPPU());
    testVBlankTiming(nes.getPPU());
    testPPUSTATUS(nes.getPPU());
    testPPUSCROLL(nes.getPPU());
    testPPUADDR(nes.getPPU());
    testWriteToggleReset(nes.getPPU());
    testPPUDATAIncrement(nes.getPPU());
    testPPUDATAReadBuffer(nes.getPPU());
    testPPUDATAPaletteRead(nes.getPPU());
    testPaletteMirroring(nes.getPPU());

    testVerticalMirroring();
    testHorizontalMirroring();
    testSingleScreenLower();
    testSingleScreenUpper();
    testCIRAMAddressMirroring3000();

    testBackgroundPixelExtraction();
    testBackgroundShifterPipeline();

    testPPUCTRLNametableSelection(nes.getPPU());

    nes.getPPU().writeVRAM(0x3F00, 0x0F);
    nes.getPPU().writeVRAM(0x3F01, 0x16);
    nes.getPPU().writeVRAM(0x3F02, 0x27);
    nes.getPPU().writeVRAM(0x3F03, 0x30);

    nes.getPPU().writeVRAM(0x3F04, 0x0F);
    nes.getPPU().writeVRAM(0x3F05, 0x13);
    nes.getPPU().writeVRAM(0x3F06, 0x2A);
    nes.getPPU().writeVRAM(0x3F07, 0x3D);

    nes.getPPU().writeVRAM(0x3F08, 0x0F);
    nes.getPPU().writeVRAM(0x3F09, 0x3B);
    nes.getPPU().writeVRAM(0x3F0A, 0x2C);
    nes.getPPU().writeVRAM(0x3F0B, 0x25);

    nes.getPPU().writeVRAM(0x3F0C, 0x0F);
    nes.getPPU().writeVRAM(0x3F0D, 0x1C);
    nes.getPPU().writeVRAM(0x3F0E, 0x11);
    nes.getPPU().writeVRAM(0x3F0F, 0x21);

    // nes.getPPU().writeVRAM(0x2000, 0x00);
    // nes.getPPU().writeVRAM(0x2001, 0x01);
    // nes.getPPU().writeVRAM(0x2002, 0x02);
    // for(int i = 0; i <= 0x03BF; ++i)
    // {
    //     nes.getPPU().writeVRAM(0x2000 + i, 0x02);
    // }

    for(int i = 0x23C0; i < 0x2400; ++i)
    {
        nes.getPPU().writeVRAM(i, 0xE4);
    }

    for(int i = 0x03C0; i < 0x0400; ++i)
    {
        nes.getPPU().writeVRAM(0x2800+i, 0xE4);
    }

    nes.getPPU().writeVRAM(0x2800 + 0x03C0 + 0x0016, 0x1B);

    // nes.getPPU().debugGenerateBackground();
    fillNametable(nes.getPPU(), 0x2000, 3);
    fillNametable(nes.getPPU(), 0x2800, 3);

    std::cout << std::hex
            << static_cast<int>(nes.getPPU().readVRAM(0x2000)) << '\n'
            << static_cast<int>(nes.getPPU().readVRAM(0x2400)) << '\n';

    
    nes.getPPU().writePPUSCROLL(0);
    nes.getPPU().writePPUSCROLL(0);

    //nes.getPPU().debugGenerateBackground();

    bool running = true;
    nes.getPPU().writePPUMASK(0x0A);


    for (int i = 0; i < 341 * 262; ++i)
    {
        nes.getPPU().clock();
    }

    fillNametable(nes.getPPU(), 0x2800, 1);

    for (int i = 0; i < 341 * 262; ++i)
    {
        nes.getPPU().clock();
    }

    nes.memoryDebug();


    while(running)
    {
        SDL_Event event;

        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
        }

        // Dibujar el framebuffer de prueba.
        renderer.renderFrame(
            nes.getPPU().getFramebuffer()
        );

        SDL_GL_SwapWindow(window);
    }

    // Liberar recursos mientras OpenGL sigue activo.
    renderer.destroy();

    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    /*
    // Pruebas de CPU e IRQ para habilitar posteriormente.

    nes.debugIRQ();

    int contador = 0;

    while(!nes.isCpuJammed())
    {
        if(contador == 25)
            nes.debugHighIRQ();

        nes.clock();
        ++contador;
    }

    nes.memoryDebug();
    */

    return 0;
}
