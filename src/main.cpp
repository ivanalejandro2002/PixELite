
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

void testPPUCTRLBackgroundPatternTable()
{
    // CHR de 8 KiB.
    auto chr = NES::NES::createTestCHR();

    // Tabla de patrones $0000: tile 0 con valor de píxel 1.
    // Plano 0 = 0xFF, plano 1 = 0x00.
    for (int row = 0; row < 8; ++row)
    {
        chr[row] = 0xFF;
        chr[row + 8] = 0x00;
    }

    // Tabla de patrones $1000: tile 0 con valor de píxel 2.
    // Plano 0 = 0x00, plano 1 = 0xFF.
    for (int row = 0; row < 8; ++row)
    {
        chr[0x1000 + row] = 0x00;
        chr[0x1000 + row + 8] = 0xFF;
    }

    NES::PPU2C02 ppu(
        chr,
        NES::CIRAM::Mirroring::Horizontal
    );

    const auto renderFirstPixel = [&ppu](uint8_t control)
    {
        ppu.reset();

        // Seleccionar el tile 0 de la primera nametable.
        ppu.writeVRAM(0x2000, 0x00);

        // Preparar colores diferentes para los valores 1 y 2.
        ppu.writeVRAM(0x3F00, 0x0F);
        ppu.writeVRAM(0x3F01, 0x01);
        ppu.writeVRAM(0x3F02, 0x21);

        ppu.writePPUCTRL(control);
        ppu.writePPUMASK(0x0A);

        // Completar pre-render y dibujar el primer scanline.
        for (int i = 0; i < 341 + 256; ++i)
            ppu.clock();

        return getPixel(ppu.getFramebuffer(), 0, 0);
    };

    const auto pixelTable0 = renderFirstPixel(0x00);
    const auto pixelTable1 = renderFirstPixel(0x10);

    // Los patrones seleccionados deben producir colores diferentes.
    assert(pixelTable0 != pixelTable1);

    std::cout
        << "PPUCTRL selección de tabla de patrones del fondo: OK\n";
}

void testPPUMASKBackgroundDisabled()
{
    auto chr = NES::NES::createTestCHR();

    NES::PPU2C02 ppu(
        chr,
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    // Seleccionar el tile 1 en la primera posición de la nametable.
    ppu.writeVRAM(0x2000, 0x01);

    // Configurar colores distinguibles.
    ppu.writeVRAM(0x3F00, 0x0F);
    ppu.writeVRAM(0x3F01, 0x01);

    ppu.writePPUCTRL(0x00);
    ppu.writePPUMASK(0x00); // Fondo deshabilitado.

    const auto before = getPixel(ppu.getFramebuffer(), 0, 0);

    // Avanzar por el pre-render y la primera scanline visible.
    for (int i = 0; i < 341 + 256; ++i)
        ppu.clock();

    const auto after = getPixel(ppu.getFramebuffer(), 0, 0);

    assert(after == before);

    std::cout << "PPUMASK: fondo deshabilitado: OK\n";
}

void testPPUMASKBackgroundLeftClipping()
{
    auto chr = NES::NES::createTestCHR();

    NES::PPU2C02 ppu(
        chr,
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    ppu.writeVRAM(0x2000, 0x01);

    ppu.writeVRAM(0x3F00, 0x0F);
    ppu.writeVRAM(0x3F01, 0x01);

    ppu.writePPUCTRL(0x00);
    ppu.writePPUMASK(0x08); // Fondo habilitado, píxeles izquierdos ocultos.

    for (int i = 0; i < 341 + 256; ++i)
        ppu.clock();

    const auto p0 = getPixel(ppu.getFramebuffer(), 0, 0);
    const auto p1 = getPixel(ppu.getFramebuffer(), 1, 0);

    // Los primeros ocho píxeles deben usar el color universal
    // del fondo, porque el fondo está oculto en esa región.
    assert(p0 == p1);

    std::cout << "PPUMASK: recorte izquierdo del fondo: OK\n";
}

void testPPUMASKBackgroundLeftVisible()
{
    auto chr = NES::NES::createTestCHR();

    NES::PPU2C02 ppu(
        chr,
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    ppu.writeVRAM(0x2000, 0x01);

    ppu.writeVRAM(0x3F00, 0x0F);
    ppu.writeVRAM(0x3F01, 0x01);

    ppu.writePPUCTRL(0x00);
    ppu.writePPUMASK(0x0A); // Fondo y región izquierda habilitados.

    for (int i = 0; i < 341 + 256; ++i)
        ppu.clock();

    const auto p0 = getPixel(ppu.getFramebuffer(), 0, 0);
    const auto p1 = getPixel(ppu.getFramebuffer(), 1, 0);

    // El patrón diagonal del tile 1 debe hacer que los píxeles
    // iniciales sean distintos.
    assert(p0 != p1);

    std::cout << "PPUMASK: región izquierda visible: OK\n";
}

void testPPUMASKColorEmphasis()
{
    const auto chrData = NES::NES::createTestCHR();

    auto renderPixel = [&](uint8_t mask)
    {
        NES::PPU2C02 ppu(
            chrData,
            NES::CIRAM::Mirroring::Horizontal
        );

        ppu.reset();

        ppu.writeVRAM(0x2000, 0x01);
        ppu.writeVRAM(0x3F00, 0x0F);
        ppu.writeVRAM(0x3F01, 0x21);

        ppu.writePPUCTRL(0x00);
        ppu.writePPUMASK(mask);

        for (int i = 0; i < 2 * 341; ++i)
            ppu.clock();

        return getPixel(ppu.getFramebuffer(), 8, 0);
    };

    const auto normal = renderPixel(0x08);
    const auto red    = renderPixel(0x28);
    const auto green  = renderPixel(0x48);
    const auto blue   = renderPixel(0x88);
    const auto all    = renderPixel(0xE8);

    // Sin énfasis
    assert(normal[0] == 84);
    assert(normal[1] == 84);
    assert(normal[2] == 84);

    // Énfasis rojo
    assert(red[0] == 84);
    assert(red[1] == 67);
    assert(red[2] == 67);

    // Énfasis verde
    assert(green[0] == 67);
    assert(green[1] == 84);
    assert(green[2] == 67);

    // Énfasis azul
    assert(blue[0] == 67);
    assert(blue[1] == 67);
    assert(blue[2] == 84);

    // Los tres bits de énfasis
    std::cout << "All emphasis RGB: "
          << static_cast<int>(all[0]) << ", "
          << static_cast<int>(all[1]) << ", "
          << static_cast<int>(all[2]) << '\n';
    assert(all[0] == 67);
    assert(all[1] == 67);
    assert(all[2] == 67);

    std::cout << "testPPUMASKColorEmphasis passed\n";
}

void testOAMDataReadWrite()
{
    const auto chrData = NES::NES::createTestCHR();

    NES::PPU2C02 ppu(
        chrData,
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    // Seleccionar la dirección 0x10.
    ppu.writeRegister(0x2003, 0x10);

    // Escribir un valor en OAM.
    ppu.writeRegister(0x2004, 0xAB);

    // OAMADDR se incrementa después de escribir.
    assert(ppu.debugOAMADDR() == 0x11);

    // Volver a seleccionar la dirección escrita.
    ppu.writeRegister(0x2003, 0x10);

    // Leer el valor almacenado.
    const uint8_t value = ppu.readRegister(0x2004);

    assert(value == 0xAB);

    std::cout << "testOAMDataReadWrite passed\n";
}

void testOAMAddressWraparound()
{
    const auto chrData = NES::NES::createTestCHR();

    NES::PPU2C02 ppu(
        chrData,
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    ppu.writeRegister(0x2003, 0xFF);
    ppu.writeRegister(0x2004, 0x5A);

    assert(ppu.debugOAMADDR() == 0x00);

    ppu.writeRegister(0x2004, 0xA5);

    assert(ppu.debugOAMADDR() == 0x01);

    // Volver a la dirección 0x00 y verificar el dato escrito.
    ppu.writeRegister(0x2003, 0x00);
    assert(ppu.readRegister(0x2004) == 0xA5);

    std::cout << "testOAMAddressWraparound passed\n";
}

void testSpriteAttributes()
{
    NES::PPU::Sprite sprite;

    sprite.attributes = 0b11100010;

    assert(sprite.palette() == 2);
    assert(sprite.behindBackground());
    assert(sprite.flipHorizontal());
    assert(sprite.flipVertical());

    std::cout << "testSpriteAttributes passed\n";
}

void testSpriteDefaultAttributes()
{
    NES::PPU::Sprite sprite;

    assert(sprite.palette() == 0);
    assert(!sprite.behindBackground());
    assert(!sprite.flipHorizontal());
    assert(!sprite.flipVertical());

    std::cout << "testSpriteDefaultAttributes passed\n";
}

void testGetSpriteFromOAM()
{
    const auto chrData = NES::NES::createTestCHR();
    NES::PPU2C02 ppu(chrData, NES::CIRAM::Mirroring::Horizontal);
    ppu.reset();

    // Configurar el segundo sprite: índice 1.
    ppu.writeRegister(0x2003, 4);
    ppu.writeRegister(0x2004, 50);         // Y
    ppu.writeRegister(0x2004, 12);          // Tile
    ppu.writeRegister(0x2004, 0b11100010); // Atributos
    ppu.writeRegister(0x2004, 100);        // X

    const auto sprite = ppu.getSprite(1);

    assert(sprite.y == 50);
    assert(sprite.tileIndex == 12);
    assert(sprite.attributes == 0b11100010);
    assert(sprite.x == 100);

    assert(sprite.palette() == 2);
    assert(sprite.behindBackground());
    assert(sprite.flipHorizontal());
    assert(sprite.flipVertical());

    std::cout << "testGetSpriteFromOAM passed\n";
}

void testSpriteScanlineEvaluation()
{
    NES::PPU::Sprite sprite;
    sprite.y = 20;

    // Sprite de 8 píxeles de altura: líneas 21 a 28.
    assert(!sprite.isOnScanline(20, 8));
    assert(sprite.isOnScanline(21, 8));
    assert(sprite.isOnScanline(28, 8));
    assert(!sprite.isOnScanline(29, 8));

    // Sprite de 16 píxeles de altura: líneas 21 a 36.
    assert(sprite.isOnScanline(21, 16));
    assert(sprite.isOnScanline(36, 16));
    assert(!sprite.isOnScanline(37, 16));

    std::cout << "testSpriteScanlineEvaluation passed\n";
}

void testSpriteScanlineLimit()
{
    const auto chrData = NES::NES::createTestCHR();
    NES::PPU2C02 ppu(
        chrData,
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    // Configurar nueve sprites que intersectan la scanline 21.
    for (uint8_t i = 0; i < 9; ++i)
    {
        ppu.writeRegister(0x2003, i * 4);
        ppu.writeRegister(0x2004, 20); // Y
        ppu.writeRegister(0x2004, i);  // Tile
        ppu.writeRegister(0x2004, 0);  // Atributos
        ppu.writeRegister(0x2004, i * 8); // X
    }

    ppu.evaluateSprites(21);

    assert(ppu.debugScanlineSpriteCount() == 8);

    assert(ppu.debugScanlineSprite(0).tileIndex == 0);
    assert(ppu.debugScanlineSprite(1).tileIndex == 1);
    assert(ppu.debugScanlineSprite(7).tileIndex == 7);

    std::cout << "testSpriteScanlineLimit passed\n";
}

void testSpriteScanlineFiltering()
{
    const auto chrData = NES::NES::createTestCHR();
    NES::PPU2C02 ppu(chrData, NES::CIRAM::Mirroring::Horizontal);
    ppu.reset();

    // Sprite 0: no intersecta la scanline 21.
    ppu.writeRegister(0x2003, 0);
    ppu.writeRegister(0x2004, 10);
    ppu.writeRegister(0x2004, 0);
    ppu.writeRegister(0x2004, 0);
    ppu.writeRegister(0x2004, 0);

    // Sprite 1: sí intersecta la scanline 21.
    ppu.writeRegister(0x2003, 4);
    ppu.writeRegister(0x2004, 20);
    ppu.writeRegister(0x2004, 1);
    ppu.writeRegister(0x2004, 0);
    ppu.writeRegister(0x2004, 8);

    ppu.evaluateSprites(21);

    assert(ppu.debugScanlineSpriteCount() == 1);
    assert(ppu.debugScanlineSprite(0).tileIndex == 1);
}

void testSpriteHeight16()
{
    const auto chrData = NES::NES::createTestCHR();
    NES::PPU2C02 ppu(chrData, NES::CIRAM::Mirroring::Horizontal);
    ppu.reset();

    // Sprite con Y = 10: ocupa las scanlines 11 a 26 en modo 8x16.
    ppu.writeRegister(0x2003, 0);
    ppu.writeRegister(0x2004, 10); // Y
    ppu.writeRegister(0x2004, 3);  // Tile
    ppu.writeRegister(0x2004, 0);  // Atributos
    ppu.writeRegister(0x2004, 0);  // X

    // Activar el bit 5 de PPUCTRL: sprites de 8x16.
    ppu.writeRegister(0x2000, 0x20);

    // La scanline 20 queda fuera del rango 11-18 de un sprite 8x8,
    // pero dentro del rango 11-26 de un sprite 8x16.
    ppu.evaluateSprites(20);

    assert(ppu.debugScanlineSpriteCount() == 1);
    assert(ppu.debugScanlineSprite(0).tileIndex == 3);

    std::cout << "testSpriteScanlineLimit16 passed\n";
}

void testSpriteOverflow()
{
    const auto chrData = NES::NES::createTestCHR();
    NES::PPU2C02 ppu(chrData, NES::CIRAM::Mirroring::Horizontal);
    ppu.reset();

    // Colocar 9 sprites en la misma scanline.
    for (uint8_t i = 0; i < 9; ++i)
    {
        ppu.writeRegister(0x2003, i * 4);
        ppu.writeRegister(0x2004, 20);     // Y
        ppu.writeRegister(0x2004, i);      // Tile
        ppu.writeRegister(0x2004, 0);      // Atributos
        ppu.writeRegister(0x2004, i * 8);  // X
    }

    ppu.evaluateSprites(21);

    // Solo deben seleccionarse 8 sprites.
    assert(ppu.debugScanlineSpriteCount() == 8);

    // PPUSTATUS bit 5 debe estar activado.
    assert((ppu.readRegister(0x2002) & 0x20) != 0);

    ppu.reset();

    assert((ppu.readRegister(0x2002) & 0x20) == 0);

    std::cout << "testSpriteOverflow passed\n";
}

void testSpriteRow()
{
    NES::PPU::Sprite sprite{
        20, // Y
        3,  // Tile
        0,  // Atributos
        40  // X
    };

    // Sin volteo vertical.
    assert(sprite.getRowInSprite(20, 8) == -1);
    assert(sprite.getRowInSprite(21, 8) == 0);
    assert(sprite.getRowInSprite(24, 8) == 3);
    assert(sprite.getRowInSprite(28, 8) == 7);
    assert(sprite.getRowInSprite(29, 8) == -1);

    // Activar volteo vertical: bit 7 de attributes.
    sprite.attributes = 0x80;

    assert(sprite.getRowInSprite(21, 8) == 7);
    assert(sprite.getRowInSprite(24, 8) == 4);
    assert(sprite.getRowInSprite(28, 8) == 0);
    assert(sprite.getRowInSprite(29, 8) == -1);
}

std::vector<uint8_t> createSpritePixelTestCHR()
{
    std::vector<uint8_t> chr(0x2000, 0);

    // Patrón 0, fila 0.
    // Bits 7, 6, 5 y 4 representan los primeros cuatro píxeles.
    chr[0] = 0b01010000; // Plano bajo: 0, 1, 0, 1
    chr[8] = 0b00110000; // Plano alto: 0, 0, 1, 1

    return chr;
}

void testSpritePixelDecoding()
{
    NES::PPU2C02 ppu(
        createSpritePixelTestCHR(),
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    NES::PPU::Sprite sprite{
        0,  // Y
        0,  // Tile
        0,  // Atributos
        0   // X
    };

    assert(ppu.getSpritePixel(sprite, 0, 0) == 0);
    assert(ppu.getSpritePixel(sprite, 1, 0) == 1);
    assert(ppu.getSpritePixel(sprite, 2, 0) == 2);
    assert(ppu.getSpritePixel(sprite, 3, 0) == 3);

    std::cout << "testSpritePixelDecoding: OK\n";
}

void testSpriteHorizontalFlip()
{
    NES::PPU2C02 ppu(
        createSpritePixelTestCHR(),
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    NES::PPU::Sprite sprite{
        0,      // Y
        0,      // Tile
        0x40,   // Atributos: volteo horizontal
        0       // X
    };

    // El volteo horizontal invierte la posición de los bits.
    // El píxel x=0 corresponde al píxel original x=7.
    assert(ppu.getSpritePixel(sprite, 0, 0) == 0);
    assert(ppu.getSpritePixel(sprite, 4, 0) == 3);
    assert(ppu.getSpritePixel(sprite, 5, 0) == 2);
    assert(ppu.getSpritePixel(sprite, 6, 0) == 1);
    assert(ppu.getSpritePixel(sprite, 7, 0) == 0);

    std::cout << "testSpriteHorizontalFlip: OK\n";
}

std::vector<uint8_t> createSpritePatternTableTestCHR()
{
    std::vector<uint8_t> chr(0x2000, 0);

    // Tabla $0000, patrón 0, fila 0:
    // Primer píxel = 1.
    chr[0] = 0b10000000;

    // Tabla $1000, patrón 0, fila 0:
    // Primer píxel = 2.
    chr[0x1000 + 8] = 0b10000000;

    return chr;
}

void testSpritePatternTableSelection()
{
    NES::PPU2C02 ppu(
        createSpritePatternTableTestCHR(),
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    NES::PPU::Sprite sprite{
        0,  // Y
        0,  // Tile
        0,  // Atributos
        0   // X
    };

    // PPUCTRL bit 3 = 0: tabla $0000.
    ppu.writeRegister(0x2000, 0x00);
    assert(ppu.getSpritePixel(sprite, 0, 0) == 1);

    // PPUCTRL bit 3 = 1: tabla $1000.
    ppu.writeRegister(0x2000, 0x08);
    assert(ppu.getSpritePixel(sprite, 0, 0) == 2);

    std::cout << "testSpritePatternTableSelection: OK\n";
}

void testSprite8x16PatternSelection()
{
    std::vector<uint8_t> chr(0x2000, 0);

    // Patrón 2: primera mitad, fila 0, píxel 0 = 1.
    chr[2 * 16] = 0b10000000;

    // Patrón 3: segunda mitad, fila 0, píxel 0 = 2.
    chr[3 * 16 + 8] = 0b10000000;

    NES::PPU2C02 ppu(
        chr,
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    NES::PPU::Sprite sprite{
        0,  // Y
        2,  // Tile
        0,  // Atributos
        0   // X
    };

    // Activar el modo 8x16.
    ppu.writeRegister(0x2000, 0x20);

    const uint8_t pixel = ppu.getSpritePixel(sprite, 0, 8);

    assert(pixel == 2);

    // Primera mitad: patrón 2.
    assert(ppu.getSpritePixel(sprite, 0, 0) == 1);

    // Segunda mitad: patrón 3.
    assert(ppu.getSpritePixel(sprite, 0, 8) == 2);

    // Las filas válidas no configuradas contienen píxeles transparentes.
    assert(ppu.getSpritePixel(sprite, 0, 7) == 0);
    assert(ppu.getSpritePixel(sprite, 0, 15) == 0);

    // Fuera de los límites del sprite.
    assert(ppu.getSpritePixel(sprite, 0, 16) == 0);
    assert(ppu.getSpritePixel(sprite, 8, 0) == 0);

    std::cout << "testSprite8x16PatternTableSelection: OK\n";
}

void testSprite8x16PatternTableSelection()
{
    std::vector<uint8_t> chr(0x2000, 0);

    // Tabla $1000, patrón 2:
    // Fila 0, plano bajo, píxel x=0 -> valor 1.
    chr[0x1000 + 2 * 16] = 0b10000000;

    // Tabla $1000, patrón 3:
    // Fila 0, plano alto, píxel x=0 -> valor 2.
    chr[0x1000 + 3 * 16 + 8] = 0b10000000;

    NES::PPU2C02 ppu(
        chr,
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    NES::PPU::Sprite sprite{
        0,  // Y
        3,  // Tile: bit 0 selecciona la tabla $1000.
        0,  // Atributos
        0   // X
    };

    // Activar el modo 8x16.
    ppu.writeRegister(0x2000, 0x20);

    // Primera mitad: patrón 2 de la tabla $1000.
    assert(ppu.getSpritePixel(sprite, 0, 0) == 1);

    // Segunda mitad: patrón 3 de la tabla $1000.
    assert(ppu.getSpritePixel(sprite, 0, 8) == 2);
}

void testSprite8x16HorizontalFlip()
{
    std::vector<uint8_t> chr(0x2000, 0);

    // Patrón 2: primera mitad del sprite.
    // Fila 0: los píxeles x=0 y x=1 valen 1 y 2.
    chr[2 * 16] = 0b11000000;
    chr[2 * 16 + 8] = 0b01000000;

    // Patrón 3: segunda mitad del sprite.
    // Fila 0: los píxeles x=0 y x=1 valen 3 y 1.
    chr[3 * 16] = 0b10000000;
    chr[3 * 16 + 8] = 0b11000000;

    NES::PPU2C02 ppu(
        chr,
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    // Activar el modo 8x16.
    ppu.writeRegister(0x2000, 0x20);

    NES::PPU::Sprite sprite{
        0,      // Y
        2,      // Tile: tabla $0000, patrones 2 y 3
        0x40,   // Volteo horizontal
        0       // X
    };

    // Primera mitad: patrón 2, fila 0.
    assert(ppu.getSpritePixel(sprite, 6, 0) == 3);
    assert(ppu.getSpritePixel(sprite, 7, 0) == 1);

    // Segunda mitad: patrón 3, fila 0.
    assert(ppu.getSpritePixel(sprite, 6, 8) == 2);
    assert(ppu.getSpritePixel(sprite, 7, 8) == 3);
}

void testSprite8x16VerticalFlip()
{
    NES::PPU::Sprite sprite{
        20,     // Y
        2,      // Tile
        0x80,   // Volteo vertical
        0       // X
    };

    constexpr uint8_t height = 16;

    // El sprite comienza en la scanline Y + 1 = 21.
    assert(sprite.getRowInSprite(21, height) == 15);
    assert(sprite.getRowInSprite(22, height) == 14);
    assert(sprite.getRowInSprite(28, height) == 8);
    assert(sprite.getRowInSprite(29, height) == 7);
    assert(sprite.getRowInSprite(36, height) == 0);

    // Fuera del área visible del sprite.
    assert(sprite.getRowInSprite(20, height) == -1);
    assert(sprite.getRowInSprite(37, height) == -1);
}

void testSprite8x16VerticalFlipIntegration()
{
    std::vector<uint8_t> chr(0x2000, 0);

    // Patrón 2: fila 0, píxel x=0 con valor 1.
    chr[2 * 16] = 0b10000000;

    // Patrón 3: fila 7, píxel x=0 con valor 2.
    chr[3 * 16 + 7 + 8] = 0b00000001;

    NES::PPU2C02 ppu(
        chr,
        NES::CIRAM::Mirroring::Horizontal
    );

    ppu.reset();

    // Activar el modo 8x16.
    ppu.writeRegister(0x2000, 0x20);

    NES::PPU::Sprite sprite{
        20,     // Y: comienza en la scanline 21.
        2,      // Patrones 2 y 3, tabla $0000.
        0x80,   // Volteo vertical.
        0       // X
    };

    auto pixelAt = [&](uint8_t scanline, uint8_t x) -> uint8_t
    {
        const int row = sprite.getRowInSprite(scanline, 16);

        if (row < 0)
            return 0;

        return ppu.getSpritePixel(
            sprite,
            x,
            static_cast<uint8_t>(row)
        );
    };

    // La fila original 0 se muestra en la scanline 36.
    assert(pixelAt(36, 0) == 1);

    // La fila original 7 del patrón 3 se muestra en la scanline 29.
    assert(pixelAt(21, 7) == 2);

    // Comprobar que las coordenadas fuera del sprite son transparentes.
    assert(pixelAt(20, 0) == 0);
    assert(pixelAt(37, 0) == 0);
}

void testSprite8x8HorizontalVerticalFlip()
{
    std::vector<uint8_t> chr(0x2000, 0);

    // Patrón 0:
    // Fila 0:
    // x=0 -> valor 1
    // x=1 -> valor 2
    chr[0] = 0b10000000;
    chr[8] = 0b01000000;

    // Fila 7:
    // x=6 -> valor 2
    // x=7 -> valor 1
    chr[7] = 0b00000011;
    chr[7 + 8] = 0b00000001;

    NES::PPU2C02 ppu(chr, NES::CIRAM::Mirroring::Horizontal);
    ppu.reset();

    // Modo 8x8, tabla de patrones $0000.
    ppu.writeRegister(0x2000, 0x00);

    NES::PPU::Sprite sprite{
        20,     // Y
        0,      // Tile
        0xC0,   // Volteo horizontal y vertical
        0       // X
    };

    auto pixelAt = [&](uint8_t scanline, uint8_t x) -> uint8_t {
        const int row = sprite.getRowInSprite(scanline, 8);

        if (row < 0)
            return 0;

        return ppu.getSpritePixel(
            sprite,
            x,
            static_cast<uint8_t>(row)
        );
    };

    // La fila original 0 se muestra en la scanline 28.
    assert(pixelAt(28, 7) == 1);
    assert(pixelAt(28, 6) == 2);

    // La fila original 7 se muestra en la scanline 21.
    assert(pixelAt(21, 0) == 3);
    assert(pixelAt(21, 1) == 1);

    // Fuera del sprite no hay píxeles.
    assert(pixelAt(20, 0) == 0);
    assert(pixelAt(29, 0) == 0);

    std::cout << "testSprite8x8HorizontalVerticalFlip: OK\n";
}

void testSprite8x16HorizontalVerticalFlip()
{
    std::vector<uint8_t> chr(0x2000, 0);

    // Tile 2, fila 0:
    // x=0 -> valor 1
    // x=1 -> valor 2
    chr[2 * 16] = 0b10000000;
    chr[2 * 16 + 8] = 0b01000000;

    // Tile 3, fila 7:
    // x=6 -> valor 2
    // x=7 -> valor 1
    chr[3 * 16 + 7] = 0b00000011;
    chr[3 * 16 + 7 + 8] = 0b00000001;

    NES::PPU2C02 ppu(chr, NES::CIRAM::Mirroring::Horizontal);
    ppu.reset();

    // Modo 8x16; tileIndex par: tabla $0000.
    ppu.writeRegister(0x2000, 0x20);

    NES::PPU::Sprite sprite{
        20,     // Y
        2,      // Tiles 2 y 3
        0xC0,   // Volteo horizontal y vertical
        0       // X
    };

    auto pixelAt = [&](uint8_t scanline, uint8_t x) -> uint8_t {
        const int row = sprite.getRowInSprite(scanline, 16);

        if (row < 0)
            return 0;

        return ppu.getSpritePixel(
            sprite,
            x,
            static_cast<uint8_t>(row)
        );
    };

    // La fila original 0 del tile 2 aparece abajo.
    assert(pixelAt(36, 7) == 1);
    assert(pixelAt(36, 6) == 2);

    // La fila original 7 del tile 3 aparece arriba.
    assert(pixelAt(21, 0) == 3);
    assert(pixelAt(21, 1) == 1);

    // Fuera del sprite no hay píxeles.
    assert(pixelAt(20, 0) == 0);
    assert(pixelAt(37, 0) == 0);

    std::cout << "testSprite8x16HorizontalVerticalFlip: OK\n";
}

void testSpritePaletteColor()
{
    std::vector<uint8_t> chr(0x2000, 0);

    NES::PPU2C02 ppu(chr, NES::CIRAM::Mirroring::Horizontal);
    ppu.reset();

    NES::PPU::Sprite sprite{
        20,     // Y
        0,      // Tile
        0x02,   // Paleta 2
        0       // X
    };

    // Paleta 2: $3F18-$3F1B.
    ppu.writeVRAM(0x3F19, 0x12);
    ppu.writeVRAM(0x3F1A, 0x23);
    ppu.writeVRAM(0x3F1B, 0x34);

    assert(ppu.getSpritePaletteColor(sprite, 0) == 0);
    assert(ppu.getSpritePaletteColor(sprite, 1) == 0x12);
    assert(ppu.getSpritePaletteColor(sprite, 2) == 0x23);
    assert(ppu.getSpritePaletteColor(sprite, 3) == 0x34);

    std::cout << "testSpritePaletteColor: OK\n";
}

void testSpritePixel()
{
    // Un píxel nuevo debe ser transparente.
    NES::PPU::SpritePixel pixel;

    assert(pixel.color == 0);
    assert(pixel.transparent);

    // Un píxel opaco con un índice de color.
    pixel.color = 0x21;
    pixel.transparent = false;

    assert(pixel.color == 0x21);
    assert(!pixel.transparent);

    // Un píxel transparente puede conservar un color sin que
    // ese valor se interprete como un color visible.
    pixel.color = 0x12;
    pixel.transparent = true;

    assert(pixel.color == 0x12);
    assert(pixel.transparent);

    std::cout << "testSpritePixel: OK\n";
}

void testSpritePixelColor()
{
    std::vector<uint8_t> chr(0x2000, 0);

    // Patrón 0, fila 0:
    // x=0 -> índice 0 (transparente)
    // x=1 -> índice 1
    chr[0] = 0b01000000;

    NES::PPU2C02 ppu(chr, NES::CIRAM::Mirroring::Horizontal);
    ppu.reset();

    // Modo 8x8, tabla de patrones $0000.
    ppu.writeRegister(0x2000, 0x00);

    // Color para el índice 1 de la paleta de sprites 2.
    ppu.writeVRAM(0x3F19, 0x12);

    NES::PPU::Sprite sprite{
        20,     // Y
        0,      // Tile
        0x02,   // Paleta 2
        0       // X
    };

    // El píxel x=0 tiene índice 0.
    const auto transparentPixel = ppu.getSpritePixelColor(sprite, 0, 0);

    assert(transparentPixel.transparent);

    // El píxel x=1 tiene índice 1.
    const auto opaquePixel = ppu.getSpritePixelColor(sprite, 1, 0);

    assert(!opaquePixel.transparent);
    assert(opaquePixel.color == 0x12);

    std::cout << "testSpritePixelColor: OK\n";
}

void testBackgroundPixel()
{
    NES::PPU::BackgroundPixel pixel;

    // El valor inicial representa un píxel transparente.
    assert(pixel.color == 0);
    assert(pixel.transparent);

    // Un píxel opaco.
    pixel.color = 0x21;
    pixel.transparent = false;

    assert(pixel.color == 0x21);
    assert(!pixel.transparent);

    // La transparencia es independiente del color almacenado.
    pixel.color = 0x12;
    pixel.transparent = true;

    assert(pixel.color == 0x12);
    assert(pixel.transparent);
    std::cout << "testBackgroundPixel: OK\n";
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
    testPPUCTRLBackgroundPatternTable();

    testPPUMASKBackgroundDisabled();
    testPPUMASKBackgroundLeftClipping();
    testPPUMASKBackgroundLeftVisible();

    testPPUMASKColorEmphasis();

    testOAMDataReadWrite();
    testOAMAddressWraparound();

    testSpriteAttributes();
    testSpriteDefaultAttributes();

    testGetSpriteFromOAM();

    testSpriteScanlineEvaluation();

    testSpriteScanlineLimit();

    testSpriteScanlineFiltering();

    testSpriteHeight16();

    testSpriteOverflow();

    testSpriteRow();

    testSpritePixelDecoding();
    testSpriteHorizontalFlip();

    testSpritePatternTableSelection();
    testSprite8x16PatternSelection();
    testSprite8x16PatternTableSelection();

    testSprite8x16HorizontalFlip();
    testSprite8x16VerticalFlip();

    testSprite8x16VerticalFlipIntegration();
    testSprite8x8HorizontalVerticalFlip();
    testSprite8x16HorizontalVerticalFlip();

    testSpritePaletteColor();
    testSpritePixel();

    testSpritePixelColor();
    testBackgroundPixel();


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



    nes.getPPU().writeVRAM(0x3F10, 0x0F);
    nes.getPPU().writeVRAM(0x3F11, 0x16);
    nes.getPPU().writeVRAM(0x3F12, 0x27);
    nes.getPPU().writeVRAM(0x3F13, 0x30);

    nes.getPPU().writeVRAM(0x3F14, 0x0F);
    nes.getPPU().writeVRAM(0x3F15, 0x13);
    nes.getPPU().writeVRAM(0x3F16, 0x2A);
    nes.getPPU().writeVRAM(0x3F17, 0x3D);

    nes.getPPU().writeVRAM(0x3F18, 0x0F);
    nes.getPPU().writeVRAM(0x3F19, 0x3B);
    nes.getPPU().writeVRAM(0x3F1A, 0x2C);
    nes.getPPU().writeVRAM(0x3F1B, 0x25);

    nes.getPPU().writeVRAM(0x3F1C, 0x0F);
    nes.getPPU().writeVRAM(0x3F1D, 0x1C);
    nes.getPPU().writeVRAM(0x3F1E, 0x11);
    nes.getPPU().writeVRAM(0x3F1F, 0x21);

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

    
    nes.getPPU().writePPUSCROLL(100);
    nes.getPPU().writePPUSCROLL(0);

    //nes.getPPU().debugGenerateBackground();

    bool running = true;
    nes.getPPU().writePPUMASK(0x1E);


    for (int i = 0; i < 341 * 262; ++i)
    {
        nes.getPPU().clock();
    }

    fillNametable(nes.getPPU(), 0x2000, 2);

    std::cout << "\n\n\n";

    nes.getPPU().writeRegister(0x2003, 0x00);
    for(int i = 0; i < 256; ++i) {
        nes.getPPU().writeRegister(0x2004, 0xFF); // Todos los Y en 0xFF (invisibles)
    }

    // 2. Ahora configuramos UN ÚNICO sprite limpio en el Índice 0 (OAMADDR = 0x00)
    nes.getPPU().writeRegister(0x2003, 0x00); 
    nes.getPPU().writeRegister(0x2004, 0x10);   // Y = 50 (en pantalla será la línea 51)
    nes.getPPU().writeRegister(0x2004, 0x00); // Tile Index = 1
    nes.getPPU().writeRegister(0x2004, 0x00); // Atributos (Paleta 0, al frente)
    nes.getPPU().writeRegister(0x2004, 0x3A);   // X = 60

    // 3. Si quisieras añadir un segundo sprite en el Índice 1, OAMADDR debe ser obligatoriamente 0x04:
    nes.getPPU().writeRegister(0x2003, 0x04); 
    nes.getPPU().writeRegister(0x2004, 80);   // Y
    nes.getPPU().writeRegister(0x2004, 0x02); // Tile
    nes.getPPU().writeRegister(0x2004, 0x00); // Atributos
    nes.getPPU().writeRegister(0x2004, 90);   // X

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
