#include <NES/PPU/PPU2C02.h>

namespace NES
{
    PPU2C02::PPU2C02(
        const std::vector<uint8_t> &chrData,
        CIRAM::Mirroring mirroring
    )
        : chr(chrData),
            ciram(mirroring)
    {
        bus.addDevice(&chr);
        bus.addDevice(&ciram);
        bus.addDevice(&palette);

        t = v = 0;
        fineX = 0;
        writeToggle = false;

        readBuffer = 0;

        ioLatch = 0;

        reset();
    }

    const Graphics::FrameBuffer& PPU2C02::getFramebuffer() const
    {
        return frameBuffer;
    }

    void PPU2C02::debugGenerateTestPattern()
    {
        for(int y = 0; y < Graphics::FrameBuffer::Height; ++y)
        {
            for(int x = 0; x < Graphics::FrameBuffer::Width; ++x)
            {
                uint8_t r = 0;
                uint8_t g = 0;
                uint8_t b = 0;

                if(x < Graphics::FrameBuffer::Width / 4)
                {
                    r = 255;
                }else if(x < Graphics::FrameBuffer::Width / 2)
                {
                    g = 255;
                }else if(x < 3 * Graphics::FrameBuffer::Width / 4)
                {
                    b = 255;
                }else
                {
                    r = g = b = 255;
                }

                frameBuffer.setPixel(x, y, r, g, b);
            }
        }
    }

    void PPU2C02::debugGenerateBackground()
    {
        // Paleta RGB de referencia para los 64 colores de la NES.
        static constexpr std::array<std::array<uint8_t, 3>, 64> nesPalette = {{
            {{ 84,  84,  84}}, {{  0,  30, 116}}, {{  8,  16, 144}}, {{ 48,   0, 136}},
            {{ 68,   0, 100}}, {{ 92,   0,  48}}, {{ 84,   4,   0}}, {{ 60,  24,   0}},
            {{ 32,  42,   0}}, {{  8,  58,   0}}, {{  0,  64,   0}}, {{  0,  60,   0}},
            {{  0,  50,  60}}, {{  0,   0,   0}}, {{  0,   0,   0}}, {{  0,   0,   0}},

            {{152, 150, 152}}, {{  8,  76, 196}}, {{ 48,  50, 236}}, {{ 92,  30, 228}},
            {{136,  20, 176}}, {{160,  20, 100}}, {{152,  34,  32}}, {{120,  60,   0}},
            {{ 84,  90,   0}}, {{ 40, 114,   0}}, {{  8, 124,   0}}, {{  0, 118,  40}},
            {{  0, 102, 120}}, {{  0,   0,   0}}, {{  0,   0,   0}}, {{  0,   0,   0}},

            {{236, 238, 236}}, {{ 76, 154, 236}}, {{120, 124, 236}}, {{176,  98, 236}},
            {{228,  84, 236}}, {{236,  88, 180}}, {{236, 106, 100}}, {{212, 136,  32}},
            {{160, 170,   0}}, {{116, 196,   0}}, {{ 76, 208,  32}}, {{ 56, 204, 108}},
            {{ 56, 180, 204}}, {{ 60,  60,  60}}, {{  0,   0,   0}}, {{  0,   0,   0}},

            {{236, 238, 236}}, {{168, 204, 236}}, {{188, 188, 236}}, {{212, 178, 236}},
            {{236, 174, 236}}, {{236, 174, 212}}, {{236, 180, 176}}, {{228, 196, 144}},
            {{204, 210, 120}}, {{180, 222, 120}}, {{168, 226, 144}}, {{152, 226, 180}},
            {{160, 214, 228}}, {{160, 162, 160}}, {{  0,   0,   0}}, {{  0,   0,   0}}
        }};

        // Base de la nametable seleccionada por PPUCTRL.
        const uint16_t nametableBase =
            0x2000 + (static_cast<uint16_t>(PPUCTRL & 0x03) * 0x0400);

        const uint16_t patternBase =
            (PPUCTRL & 0x10) ? 0x1000 : 0x0000;

        // Decodificar la dirección temporal t.
        const int coarseX = t & 0x001F;
        const int coarseY = (t >> 5) & 0x001F;
        const int nametableX = (t >> 10) & 0x01;
        const int nametableY = (t >> 11) & 0x01;
        const int fineY = (t >> 12) & 0x07;

        // Coordenadas de scroll iniciales, expresadas en píxeles.
        const int scrollX = coarseX * 8 + fineX;
        const int scrollY = coarseY * 8 + fineY;

        // Selección de nametable inicial.
        const int baseNametableX = PPUCTRL & 0x01;
        const int baseNametableY = (PPUCTRL >> 1) & 0x01;

        // El scroll se calcula sobre el espacio de dos nametables
        // por eje. La visualización usa las coordenadas resultantes
        // para elegir tiles, patrones y atributos.
        const int originX = baseNametableX * 256 + scrollX;
        const int originY = baseNametableY * 240 + scrollY;

        (void)nametableX;
        (void)nametableY;

        for (int y = 0; y < Graphics::FrameBuffer::Height; ++y)
        {
            for (int x = 0; x < Graphics::FrameBuffer::Width; ++x)
            {
                // Coordenadas desplazadas en el espacio de nametables.
                const int sx = (x + originX) % 512;
                const int sy = (y + originY) % 480;

                // Elegir la nametable lógica según las coordenadas.
                const int ntX = sx / 256;
                const int ntY = sy / 240;

                const int localX = sx % 256;
                const int localY = sy % 240;

                const uint16_t currentNametable =
                    0x2000 + ntY * 0x0800 + ntX * 0x0400;

                const int tileX = localX / 8;
                const int tileY = localY / 8;

                const int pixelX = localX % 8;
                const int pixelY = localY % 8;

                // 1. Obtener el identificador del tile.
                const uint16_t tileAddress =
                    currentNametable + tileY * 32 + tileX;

                const uint8_t tileIndex = readVRAM(tileAddress);

                // 2. Leer los dos planos de bits del patrón.
                const uint16_t patternAddress =
                    patternBase +
                    static_cast<uint16_t>(tileIndex) * 16 +
                    pixelY;

                const uint8_t plane0 = readVRAM(patternAddress);
                const uint8_t plane1 = readVRAM(patternAddress + 8);

                const int bit = 7 - pixelX;

                const uint8_t pixel =
                    ((plane0 >> bit) & 0x01) |
                    (((plane1 >> bit) & 0x01) << 1);

                // 3. Obtener la subpaleta desde la tabla de atributos.
                const uint16_t attributeAddress =
                    currentNametable + 0x03C0 +
                    (tileY / 4) * 8 +
                    (tileX / 4);

                const uint8_t attribute = readVRAM(attributeAddress);

                const int shift =
                    ((tileY % 4) / 2) * 4 +
                    ((tileX % 4) / 2) * 2;

                const uint8_t backgroundPalette =
                    (attribute >> shift) & 0x03;

                // 4. El píxel 0 utiliza el color universal de fondo.
                const uint16_t paletteAddress =
                    (pixel == 0)
                        ? 0x3F00
                        : 0x3F00 + backgroundPalette * 4 + pixel;

                const uint8_t colorIndex =
                    readVRAM(paletteAddress) & 0x3F;

                // 5. Convertir el índice a RGB.
                const auto& color = nesPalette[colorIndex];

                frameBuffer.setPixel(
                    x,
                    y,
                    color[0],
                    color[1],
                    color[2]
                );
            }
        }
    }

    uint8_t PPU2C02::readVRAM(uint16_t addr)
    {
        return bus.read(addr & 0x3FFF);
    }

    void PPU2C02::writeVRAM(uint16_t addr, uint8_t value)
    {
        bus.write(addr & 0x3FFF, value);
    }

    uint8_t PPU2C02::readRegister(uint16_t addr)
    {
        uint8_t value = ioLatch;

        switch(addr & 0x0007)
        {
            // PPUSTATUS
            case 0x0002:
                value = readPPUSTATUS();

                value = (value & 0xE0) | (ioLatch & 0x1F);
                break;

            // OAMDATA
            case 0x0004:
                value = readOAMDATA();
                break;

            // PPUDATA
            case 0x0007:
                value = readPPUDATA();
                break;

            default:
                break;
        }

        ioLatch = value;
        return value;
    }

    void PPU2C02::writeRegister(uint16_t addr, uint8_t value)
    {
        ioLatch = value;

        switch(addr & 0x0007)
        {
            // PPUCTRL
            case 0x0000:
                writePPUCTRL(value);
                break;
            
            // PPUMASK
            case 0x0001:
                writePPUMASK(value);
                break;

            // PPUSTATUS
            case 0x0002:
                break;

            // OAMADDR
            case 0x0003:
                writeOAMADDR(value);
                break;

            // OAMDATA
            case 0x0004:
                writeOAMDATA(value);
                break;

            // PPUSCROLL
            case 0x0005:
                writePPUSCROLL(value);
                break;

            // PPUADDR
            case 0x0006:
                writePPUADDR(value);
                break;

            // PPUDATA
            case 0x0007:
                writePPUDATA(value);
                break;
        }
    }

    // $2000 -> PPUCTRL

    void PPU2C02::writePPUCTRL(uint8_t value)
    {
        PPUCTRL = value;

        t &= 0xF3FF;
        t |= (static_cast<uint16_t>(value) & 0x03) << 10;
    }

    // $2001 -> PPUMASK

    void PPU2C02::writePPUMASK(uint8_t value)
    {
        PPUMASK = value;
    }

    // $2002 -> PPUSTATUS

    uint8_t PPU2C02::readPPUSTATUS()
    {
        uint8_t value = PPUSTATUS;

        PPUSTATUS &= 0x7F;
        writeToggle = false;

        return value;
    }

    // $2003 -> OAMADDR

    void PPU2C02::writeOAMADDR(uint8_t value)
    {
        OAMADDR = value;
    }

    // $2004 -> OAMDATA

    uint8_t PPU2C02::readOAMDATA()
    {
        return OAM[OAMADDR];
    }

    void PPU2C02::writeOAMDATA(uint8_t value)
    {
        OAM[OAMADDR] = value;
        OAMADDR++;
    }

    // $2005 -> PPUSCROLL

    void PPU2C02::writePPUSCROLL(uint8_t value)
    {
        if(!writeToggle)
        {
            fineX = value & 0x07;

            t &= 0xFFE0;
            t |= (static_cast<uint16_t>(value) >> 3) & 0x001F;

            writeToggle = true;
        }else
        {
            t &= 0x8C1F;
            t |= (static_cast<uint16_t>(value) & 0x07) << 12;
            t |= (static_cast<uint16_t>(value) & 0xF8) << 2;

            writeToggle = false;
        }
    }

    // $2006 -> PPUADDR

    void PPU2C02::writePPUADDR(uint8_t value)
    {
        if(!writeToggle)
        {
            t &= 0x00FF;
            t |= (static_cast<uint16_t>(value) & 0x3F) << 8;

            writeToggle = true;
        }else
        {
            t &= 0x7F00;
            t |= value;

            v = t;

            writeToggle = false;
        }
    }

    // $2007 -> PPUDATA

    uint8_t PPU2C02::readPPUDATA()
    {
        uint16_t addr = v & 0x3FFF;
        uint8_t value;

        if(addr >= 0x3F00)
        {
            value = readVRAM(addr);
            readBuffer = readVRAM((addr - 0x1000) & 0x3FFF);
        }
        else
        {
            value = readBuffer;
            readBuffer = readVRAM(addr);
        }

        const uint16_t increment = (PPUCTRL & 0x04) ? 32 : 1;
        v = (v + increment) & 0x7FFF;

        return value;
    }

    void PPU2C02::writePPUDATA(uint8_t value)
    {
        writeVRAM(v & 0x3FFF, value);
        const uint16_t increment = (PPUCTRL & 0x04) ? 32 : 1;
        v = (v + increment) & 0x7FFF;
    }

    uint16_t PPU2C02::getVRAMAddress() const
    {
        return v;
    }

    void PPU2C02::reset()
    {
        PPUCTRL = 0;
        PPUMASK = 0;
        PPUSTATUS = 0;
        OAMADDR = 0;

        v = 0;
        t = 0;
        fineX = 0;
        writeToggle = false;

        readBuffer = 0;
        ioLatch = 0;

        OAM.fill(0);
    }

    uint8_t PPU2C02::debugPPUCTRL() const {return PPUCTRL;}
    uint8_t PPU2C02::debugPPUMASK() const {return PPUMASK;}
    uint8_t PPU2C02::debugPPUSTATUS() const {return PPUSTATUS;}
    uint8_t PPU2C02::debugOAMADDR() const {return OAMADDR;}
    uint8_t PPU2C02::debugOAMDATA() const {return OAM[OAMADDR];}

    uint16_t PPU2C02::debugV() const {return v;}
    uint16_t PPU2C02::debugT() const {return t;}
    uint8_t PPU2C02::debugX() const {return fineX;}
    bool PPU2C02::debugW() const {return writeToggle;}
}