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

    void PPU2C02::evaluateSprites(uint8_t scanline)
    {
        currentScanlineSpriteCount = 0;
        
        if(scanline < 0 || scanline >= 240) return;
        
        const uint8_t spriteHeight = (PPUCTRL & 0x20) ? 16:8;

        for(uint8_t i = 0; i < 64; ++i)
        {
            const NES::PPU::Sprite sprite = getSprite(i);

            if(!sprite.isOnScanline(
                static_cast<uint8_t>(scanline),
                spriteHeight))
            {
                continue;
            }

            if(currentScanlineSpriteCount >= MAX_SPRITES_PER_SCANLINE)
            {
                PPUSTATUS |= 0x20;
                break;
            }

            currentScanlineSprites[currentScanlineSpriteCount] = sprite;
            ++currentScanlineSpriteCount;
        }
    }

    void PPU2C02::evaluateSingleSprite(uint8_t index, int targetScanline)
    {
        const NES::PPU::Sprite sprite = getSprite(index);

        const uint8_t spriteHeight = (PPUCTRL & 0x20)? 16 : 8;

        if(sprite.isOnScanline(static_cast<uint8_t>(targetScanline), spriteHeight))
        {
            if(nextScanlineSpriteCount < MAX_SPRITES_PER_SCANLINE)
            {
                nextScanlineSprites[nextScanlineSpriteCount] = sprite;
                ++nextScanlineSpriteCount;
            }
            else
            {
                PPUSTATUS |= 0x20;
            }
        }
    }

    uint8_t PPU2C02::getSpritePixel(
        const NES::PPU::Sprite& sprite,
        uint8_t x,
        uint8_t row
    )
    {
        const bool is8x16 = (PPUCTRL & 0x20) != 0;
        const uint8_t spriteHeight = is8x16 ? 16 : 8;

        if (x >= 8 || row >= spriteHeight)
            return 0;

        uint16_t patternBase;
        uint8_t tileIndex;
        uint8_t tileRow;

        if(is8x16)
        {
            // En modo 8x16, el bit 0 selecciona la pattern table
            patternBase = (sprite.tileIndex & 0x01) ? 0x1000 : 0x0000;

            // El primer patrón tiene índice par siempre.
            tileIndex = sprite.tileIndex & 0xFE;

            // Aquí pertenece el segundo patrón
            if(row >= 8)
            {
                ++tileIndex;
                tileRow = row - 8;
            }
            else
            {
                tileRow = row;
            }
        }
        else
        {
            // En modo 8x8, el bit 3 de PPUCTRL selecciona la pattern table
            patternBase = (PPUCTRL & 0x08) ? 0x1000: 0x0000;

            tileIndex = sprite.tileIndex;
            tileRow = row;
        }

        const uint16_t tileAddress =
            patternBase + static_cast<uint16_t>(tileIndex) * 16;

        // Aplicar volteo horizontal.
        const uint8_t pixelX = sprite.flipHorizontal() ? static_cast<uint8_t>(7 - x) : x;

        // Leer los dos planos de bits.
        const uint16_t lowAddress = tileAddress + tileRow;
        const uint16_t highAddress = tileAddress + tileRow + 8;

        const uint8_t lowByte = readVRAM(lowAddress);
        const uint8_t highByte = readVRAM(highAddress);

        const uint8_t bit = 7 - pixelX;

        const uint8_t lowBit = (lowByte >> bit) & 1;
        const uint8_t highBit = (highByte >> bit) & 1;

        return lowBit | (highBit << 1);
    }

    NES::PPU::SpritePixel PPU2C02::getSpritePixelColor(
        const NES::PPU::Sprite& sprite,
        uint8_t x,
        uint8_t row
    )
    {
        const uint8_t pixelValue = getSpritePixel(sprite, x, row);

        // El índice 0 representa un píxel transparente.
        if (pixelValue == 0)
            return {0, true};

        const uint8_t color = getSpritePaletteColor(sprite, pixelValue);

        return {color, false};
    }

    uint8_t PPU2C02::getSpritePaletteColor(
        const NES::PPU::Sprite& sprite,
        uint8_t pixelValue
    )
    {
        // El índice 0 representa transparencia.
        if (pixelValue == 0)
            return 0;

        const uint8_t paletteIndex = sprite.attributes & 0x03;

        const uint16_t paletteAddress =
            0x3F10 + paletteIndex * 4 + pixelValue;

        return readVRAM(paletteAddress);
    }

    NES::PPU::BackgroundPixel PPU2C02::getBackgroundPixel()
    {
        const int x = static_cast<int>(cycle) - 1;

        if (x < 0 || x >= Graphics::FrameBuffer::Width)
            return {0, true};

        uint8_t pixel = 0;
        uint8_t backgroundPalette = 0;

        const bool showBackGround = (PPUMASK & 0x08) != 0;
        const bool showLeftBackGround = (PPUMASK & 0x02) != 0;

        if (showBackGround && (x >= 8 || showLeftBackGround))
        {
            const uint16_t mux = static_cast<uint16_t>(0x8000 >> fineX);

            pixel = ((bgPatternShiftLow & mux) ? 1 : 0) |
                    ((bgPatternShiftHigh & mux) ? 2 : 0);

            backgroundPalette =
                ((bgAttributeShiftLow & mux) ? 1 : 0) |
                ((bgAttributeShiftHigh & mux) ? 2 : 0);
        }

        const uint16_t paletteAddress =
            (pixel == 0)
                ? 0x3F00
                : static_cast<uint16_t>(
                    0x3F00 + backgroundPalette * 4 + pixel);

        const uint8_t colorIndex = readVRAM(paletteAddress) & 0x3F;

        return {
            colorIndex,
            pixel == 0
        };
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

    void PPU2C02::clock()
    {
        ++cycle;

        const bool renderingEnabled = (PPUMASK & 0x18) != 0;
        const bool visibleScanLine = scanline >= 0 && scanline < 240;
        const bool preRenderScanLine = scanline == -1;

        const bool renderScanLine = visibleScanLine || preRenderScanLine;

        if(scanline == 241 && cycle == 1)
        {
            //Vblank
            PPUSTATUS |= 0x80;
        }

        if(preRenderScanLine && cycle == 1)
        {
            //Limpiar Vblank y flags de sprite
            PPUSTATUS &= 0x1F;
            nextScanlineSpriteCount = 0;
        }

        if(renderScanLine && renderingEnabled)
        {

            if(cycle == 1)
            {
                for (int i = 0; i < nextScanlineSpriteCount; ++i)
                    currentScanlineSprites[i] = nextScanlineSprites[i];
                
                currentScanlineSpriteCount = nextScanlineSpriteCount;
                nextScanlineSpriteCount = 0;
                spriteEvaluationIndex = 0;
            }

            if(cycle >= 65 && cycle <= 256)
            {
                oddCycle = !oddCycle;

                if(oddCycle && spriteEvaluationIndex < 64)
                {
                    evaluateSingleSprite(spriteEvaluationIndex, scanline + 1);
                    ++spriteEvaluationIndex;
                }
            }

            const bool backgroundFetchCycle = 
                (cycle >= 1 && cycle <= 256) ||
                (cycle >= 321 && cycle <= 336);

            if(visibleScanLine && cycle >= 1 && cycle <= 256)
                renderPixel();

            if(backgroundFetchCycle)
            {
                updateBackgroundShifters();
                backgroundFetch();
            }

            if(cycle == 256) incrementY();

            if(cycle == 257) transferAddressX();

            if(preRenderScanLine && cycle >= 280 && cycle <= 304) transferAddressY();
        }


        if(cycle >= 341)
        {
            cycle = 0;
            ++scanline;

            if(scanline > 260)
                scanline = -1;
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

    NES::PPU::Sprite PPU2C02::getSprite(uint8_t index) const
    {
        const uint16_t offset = static_cast<uint16_t>(index) * 4;

        return {
            OAM[offset],
            OAM[offset + 1],
            OAM[offset + 2],
            OAM[offset + 3]
        };
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

        bgPatternShiftLow = 0;
        bgPatternShiftHigh = 0;

        bgAttributeShiftLow = 0;
        bgAttributeShiftHigh = 0;

        nextTileID = 0;
        nextTileAttribute = 0;
        nextTileLow = 0;
        nextTileHigh = 0;

        scanline = -1;
        cycle = 0;

        spriteEvaluationIndex = 0;
        oddCycle = false;
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

    uint16_t PPU2C02::debugCycle() const {return cycle;}
    int16_t PPU2C02::debugScanline() const {return scanline;}

    uint8_t PPU2C02::debugScanlineSpriteCount() const {return currentScanlineSpriteCount;}

    const NES::PPU::Sprite& PPU2C02::debugScanlineSprite(uint8_t index) const
    {
        return currentScanlineSprites[index];
    }

    void PPU2C02::incrementCoarseX()
    {
        if(!(PPUMASK & 0x18)) return;

        if((v & 0x001F) == 31)
        {
            v &= ~0x001F;
            v ^= 0x0400;
        }else
        {
            ++v;
        }
    }

    void PPU2C02::incrementY()
    {
        if(!(PPUMASK & 0x18))return;

        if((v & 0x7000) != 0x7000)
        {
            v += 0x1000;
            return;
        }

        v &= ~0x7000;
        uint16_t coarseY = (v & 0x03E0) >> 5;
        if(coarseY == 29)
        {
            coarseY = 0;
            v ^= 0x0800;
        }else if(coarseY == 31)
        {
            coarseY = 0;
        }else
        {
            ++coarseY;
        }

        v = (v & ~0x03E0) | (coarseY << 5);
    }

    void PPU2C02::transferAddressX()
    {
        if(!(PPUMASK & 0x18)) return;

        v = (v & ~0x041F) | (t & 0x041F);
    }

    void PPU2C02::transferAddressY()
    {
        if(!(PPUMASK & 0x18)) return;

        v = (v & ~0x7BE0) | (t & 0x7BE0);
    }


    void PPU2C02::loadBackgroundShifters()
    {
        bgPatternShiftLow = (bgPatternShiftLow & 0xFF00) | nextTileLow;

        bgPatternShiftHigh = (bgPatternShiftHigh & 0xFF00) | nextTileHigh;

        const uint8_t attributeLow = (nextTileAttribute & 0x01) ? 0xFF : 0x00;

        const uint8_t attributeHigh = (nextTileAttribute & 0x02) ? 0xFF : 0x00;

        bgAttributeShiftLow = (bgAttributeShiftLow & 0xFF00) | attributeLow;
        bgAttributeShiftHigh = (bgAttributeShiftHigh & 0xFF00) | attributeHigh;
    }

    void PPU2C02::updateBackgroundShifters()
    {
        bgPatternShiftLow <<= 1;
        bgPatternShiftHigh <<= 1;

        bgAttributeShiftLow <<= 1;
        bgAttributeShiftHigh <<= 1;
    }

    void PPU2C02::backgroundFetch()
    {
        switch(cycle & 0x0007)
        {
            case 1:
                {
                    //Lectura ID de nametable
                    const uint16_t address = 0x2000 | (v & 0x0FFF);

                    nextTileID = readVRAM(address);
                    break;
                }
            
            case 3:
                //Lectura de tabla de atributos
                {
                    const uint16_t address = 
                        0x23C0 | (v & 0x0C00) | ((v>>4) & 0x0038) | ((v>>2) & 0x0007);

                    const uint8_t attribute = readVRAM(address);

                    const uint8_t shift =
                        static_cast<uint8_t>(((v>>4) & 0x04) | (v & 0x02));

                    nextTileAttribute = (attribute >> shift) & 0x03;
                    break;
                }

            case 5:
                //Lectura baja CHR
                {
                    const uint16_t fineY = (v >> 12) & 0x07;

                    const uint16_t patternBase = (PPUCTRL & 0x10)? 0x1000 : 0x0000;

                    const uint16_t address =
                        patternBase + static_cast<uint16_t>(nextTileID) * 16 + fineY;

                    nextTileLow = readVRAM(address);
                    break;
                }

            case 7:
                //Lectura plano alto CHR
                {
                    const uint16_t fineY = (v >> 12) & 0x07;

                    const uint16_t patternBase = (PPUCTRL & 0x10)? 0x1000 : 0x0000;

                    const uint16_t address = 
                        patternBase + static_cast<uint16_t>(nextTileID) * 16 + fineY + 8;

                    nextTileHigh = readVRAM(address);
                    break;
                }

            case 0:
                {
                    loadBackgroundShifters();

                    incrementCoarseX();
                    break;
                }
        }
    }

    void PPU2C02::renderBackgroundPixel()
    {
        const int x = static_cast<int>(cycle) - 1;
        const int y = static_cast<int>(scanline);

        if(x < 0 || x >= Graphics::FrameBuffer::Width ||
            y < 0 || y >= Graphics::FrameBuffer::Height)
            return;

        uint8_t pixel = 0;
        uint8_t backgroundPalette = 0;

        const bool showBackGround = (PPUMASK & 0x08) != 0;
        const bool showLeftBackGround = (PPUMASK & 0x02) != 0;

        if(showBackGround && (x >= 8 || showLeftBackGround))
        {
            const uint16_t mux = static_cast<uint16_t> (0x8000 >> fineX);

            pixel = ((bgPatternShiftLow & mux) ? 1: 0) |
                    ((bgPatternShiftHigh & mux)? 2: 0);

            backgroundPalette = ((bgAttributeShiftLow & mux) ? 1: 0) |
                                ((bgAttributeShiftHigh & mux)? 2: 0);
        }

        const uint16_t paletteAddress = 
            (pixel == 0)? 0x3F00: static_cast<uint16_t>(0x3F00 + backgroundPalette * 4 + pixel);

        uint8_t colorIndex = readVRAM(paletteAddress) & 0x3F;

        if(PPUMASK & 0x01) colorIndex &= 0x30;
        
        const auto& color = nesPalette[colorIndex];

        const auto emphasizedColor = applyColorEmphasis(color);

        frameBuffer.setPixel(x, y, emphasizedColor[0], emphasizedColor[1], emphasizedColor[2]);
    }

    void PPU2C02::renderPixel()
    {
        const int x = static_cast<int>(cycle) - 1;
        const int y = static_cast<int>(scanline);

        if (x < 0 || x >= Graphics::FrameBuffer::Width ||
            y < 0 || y >= Graphics::FrameBuffer::Height)
            return;

        const bool showBackground = (PPUMASK & 0x08) != 0;
        const bool showLeftBackground = (PPUMASK & 0x02) != 0;
        const bool showSprites = (PPUMASK & 0x10) != 0;
        const bool showLeftSprites = (PPUMASK & 0x04) != 0;

        // Aplicar máscaras de recorte izquierdo (primeras 8 columnas)
        bool bgVisible = showBackground && (x >= 8 || showLeftBackground);
        bool spriteVisible = showSprites && (x >= 8 || showLeftSprites);

        // 1. Obtener píxel de fondo
        NES::PPU::BackgroundPixel bgPixel = getBackgroundPixel();
        if (!bgVisible) bgPixel.transparent = true;

        // 2. Obtener píxel de sprite prioritario
        EvaluatedSpritePixel spritePixel = getTopSpritePixel(x, y);
        if (!spriteVisible) spritePixel.transparent = true;

        // 3. Detección de Sprite 0 Hit
        if (spritePixel.isSpriteZero && !spritePixel.transparent && !bgPixel.transparent && showBackground && showSprites)
        {
            if (x < 255 && x >= 0)
            {
                PPUSTATUS |= 0x40; // Activar bit de Sprite 0 Hit
            }
        }

        uint8_t finalColorIndex = 0;

        // 4. Multiplexación de prioridades entre Fondo y Sprites
        if (bgPixel.transparent && spritePixel.transparent)
        {
            // Ambos transparentes: Usar el color universal de fondo ($3F00)
            finalColorIndex = readVRAM(0x3F00) & 0x3F;
        }
        else if (bgPixel.transparent && !spritePixel.transparent)
        {
            // Fondo transparente, Sprite opaco -> Se dibuja el sprite
            finalColorIndex = spritePixel.colorIndex;
        }
        else if (!bgPixel.transparent && spritePixel.transparent)
        {
            // Fondo opaco, Sprite transparente -> Se dibuja el fondo
            finalColorIndex = bgPixel.color; // bgPixel.color ya contiene el color resuelto
        }
        else
        {
            // Ambos opacos: Evaluar la prioridad (behindBackground)
            if (spritePixel.behindBackground)
            {
                finalColorIndex = bgPixel.color;
            }
            else
            {
                finalColorIndex = spritePixel.colorIndex;
            }
        }

        // 5. Aplicar escala de grises y énfasis de color
        if (PPUMASK & 0x01) finalColorIndex &= 0x30;

        const auto& color = nesPalette[finalColorIndex];
        const auto emphasizedColor = applyColorEmphasis(color);

        frameBuffer.setPixel(x, y, emphasizedColor[0], emphasizedColor[1], emphasizedColor[2]);
    }

    std::array<uint8_t, 3> PPU2C02::applyColorEmphasis(
        const std::array<uint8_t, 3>& color) const
    {
        constexpr float attenuation = 0.8f;

        const bool emphasizeRed   = (PPUMASK & 0x20) != 0;
        const bool emphasizeGreen = (PPUMASK & 0x40) != 0;
        const bool emphasizeBlue  = (PPUMASK & 0x80) != 0;

        auto result = color;

        if (emphasizeRed || emphasizeGreen || emphasizeBlue)
        {
            if (!emphasizeRed || (emphasizeGreen && emphasizeBlue))
                result[0] = static_cast<uint8_t>(result[0] * attenuation);

            if (!emphasizeGreen || (emphasizeRed && emphasizeBlue))
                result[1] = static_cast<uint8_t>(result[1] * attenuation);

            if (!emphasizeBlue || (emphasizeRed && emphasizeGreen))
                result[2] = static_cast<uint8_t>(result[2] * attenuation);
        }

        return result;
    }
}