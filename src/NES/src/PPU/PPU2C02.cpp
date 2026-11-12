#include <NES/PPU/PPU2C02.h>

namespace NES
{
    PPU2C02::PPU2C02(
        Cartridge *cartridge,
        Mirroring mirroring
    )
        : chrBridge(cartridge),
            ciram(mirroring)
    {
        bus.addDevice(&chrBridge);
        bus.addDevice(&ciram);
        bus.addDevice(&palette);

        t = v = 0;
        fineX = 0;
        writeToggle = false;

        readBuffer = 0;

        ioLatch = 0;

        reset();
    }

    PPU2C02::PPU2C02(const std::vector<uint8_t> &chrData, Mirroring mirroring)
        : chrBridge(nullptr),
          ciram(mirroring)
    {
        chr = std::make_unique<CHRROM>(chrData);
        bus.addDevice(chr.get());
        bus.addDevice(&ciram);
        bus.addDevice(&palette);

        t = v = 0;
        fineX = 0;
        writeToggle = false;

        readBuffer = 0;

        ioLatch = 0;

        reset();
    }

    void PPU2C02::connectMainBus(Bus *b) {mainBus = b;}

    void PPU2C02::oamDma(uint8_t pageValue)
    {
        uint16_t baseAddress = static_cast<uint16_t>(pageValue) << 8;
        for(int i = 0; i < 256; ++i)
        {
            uint8_t data = mainBus->read(baseAddress + i);

            uint8_t targetIndex = (OAMADDR + i) & 0xFF;
            OAM[targetIndex] = data;
        }
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

    void PPU2C02::evaluateSingleSprite(uint8_t oamOffset, int targetScanline)
    {
        const uint8_t spriteHeight = (PPUCTRL & 0x20)? 16 : 8;

        if(nextScanlineSpriteCount < MAX_SPRITES_PER_SCANLINE)
        {
            // Construir sprite desde el offset exacto de OAM (soporta OAM desalineada)
            NES::PPU::Sprite sprite = {
                OAM[oamOffset & 0xFF],
                OAM[(oamOffset + 1) & 0xFF],
                OAM[(oamOffset + 2) & 0xFF],
                OAM[(oamOffset + 3) & 0xFF],
                (spriteEvaluationIndex == 0) // El primer sprite evaluado es el candidato a Sprite Zero
            };

            if (sprite.isOnScanline(static_cast<uint8_t>(targetScanline), spriteHeight))
            {
                nextScanlineSprites[nextScanlineSpriteCount] = sprite;
                ++nextScanlineSpriteCount;
            }
        }
        else
        {
            uint8_t evalY = OAM[(oamOffset + oamM) & 0xFF];

            int startLine = static_cast<int>(evalY) + 1;
            int row = targetScanline - startLine;

            bool onLine = (row >= 0 && row < spriteHeight);

            if(onLine)
            {
                // Encontrada una colisión falso positivo por desalineación
                PPUSTATUS |= 0x20;

                oamM = (oamM + 1) & 0x03;
            }
            else
            {
                // Incremento diagonal por bug de desbordamiento de sprites
                oamM = (oamM + 1) & 0x03;
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

    uint8_t PPU2C02::getSpritePixel(
        const NES::PPU::Sprite& sprite,
        uint8_t x,
        uint8_t row,
        int spriteIndex
    )
    {
        const bool is8x16 = (PPUCTRL & 0x20) != 0;
        const uint8_t spriteHeight = is8x16 ? 16 : 8;

        if (x >= 8 || row >= spriteHeight || spriteIndex < 0 || spriteIndex >= 8)
            return 0;

        // Obtenemos los bytes que la PPU ya leyó previamente en la ventana 257-320
        const uint8_t lowByte = spritePatternLowBytes[spriteIndex];
        const uint8_t highByte = spritePatternHighBytes[spriteIndex];

        // Aplicar volteo horizontal
        const uint8_t pixelX = sprite.flipHorizontal() ? static_cast<uint8_t>(7 - x) : x;

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

    CIRAM& PPU2C02::getCiram(){return ciram;}

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

        if (nmiDelay > 0)
        {
            nmiDelay--;
            if (nmiDelay == 0 && (PPUSTATUS & 0x80) && (PPUCTRL & 0x80))
            {
                mainBus->setNMILine(false);
            }
        }

        if (openBusTimer > 0)
        {
            openBusTimer--;
            if (openBusTimer == 0)
        {
                ioLatch = 0x00; // El bus abierto decae a cero tras 1 segundo sin uso
            }
        }

        const bool renderingEnabled = (PPUMASK & 0x18) != 0;
        const bool visibleScanLine = scanline >= 0 && scanline < 240;
        const bool preRenderScanLine = scanline == -1;

        const bool renderScanLine = visibleScanLine || preRenderScanLine;

        if (scanline == 241 && cycle == 1)
        {
            if (!suppressVBlankFlag)
            {
                PPUSTATUS |= 0x80;
                if (PPUCTRL & 0x80)
                {
                    nmiDelay = 2; // Dispara exactamente en dot 2
                }
            }
            suppressVBlankFlag = false;
        }

        if(preRenderScanLine)
        {
            if(cycle == 1)
            {
                // Limpiar Vblank y liberar NMI en cycle 1 (pasa VBlank End)
                PPUSTATUS &= 0x7F;
                nextScanlineSpriteCount = 0;
                oamM = 0;
                mainBus->setNMILine(true);
            }
            else if(cycle == 2)
            {
                // Limpiar Sprite 0 Hit y Sprite Overflow en cycle 2 (pasa Flag Timing)
                PPUSTATUS &= 0x1F;
            }
        }

        if(renderScanLine && renderingEnabled)
        {
            if(cycle == 1)
            {
                for (int i = 0; i < nextScanlineSpriteCount; ++i)
                {
                    currentScanlineSprites[i] = nextScanlineSprites[i];
                    spritePatternLowBytes[i] = nextSpritePatternLowBytes[i];
                    spritePatternHighBytes[i] = nextSpritePatternHighBytes[i];
                }
                
                currentScanlineSpriteCount = nextScanlineSpriteCount;
                
                // Limpiar el resto de los búferes activos si hay menos sprites
                for (int i = nextScanlineSpriteCount; i < 8; ++i)
                {
                    spritePatternLowBytes[i] = 0;
                    spritePatternHighBytes[i] = 0;
                }

                nextScanlineSpriteCount = 0;
                spriteEvaluationIndex = 0;
            }

            if(cycle == 65)
            {
                oamM = 0; // Resetear m al inicio de la evaluación de sprites de cada scanline
            }

            if(cycle >= 65 && cycle <= 256)
            {
                oddCycle = !oddCycle;

                if(oddCycle && spriteEvaluationIndex < 64)
                {
                    uint8_t oamOffset;

                    // Si OAMADDR no está desalineado (caso estándar de casi todos los juegos):
                    if ((OAMADDR & 0x03) == 0)
                    {
                        oamOffset = (OAMADDR + spriteEvaluationIndex * 4) & 0xFF;
                    }
                    else
                    {
                        // Manejo de OAM desalineada (Misaligned OAM):
                        if (spriteEvaluationIndex == 0)
                        {
                            oamOffset = OAMADDR;
                        }
                        else
                        {
                            // Comprobar si el primer objeto evaluado estaba en rango
                            uint8_t firstY = OAM[OAMADDR];
                            int startLine = static_cast<int>(firstY) + 1;
                            int row = (scanline + 1) - startLine;
                            const uint8_t spriteHeight = (PPUCTRL & 0x20) ? 16 : 8;
                            bool firstOnLine = (row >= 0 && row < spriteHeight);

                            uint8_t realignedBase;
                            if (firstOnLine)
                            {
                                // Si estaba en rango, se mantuvo desalineado
                                realignedBase = (OAMADDR + 4) & 0xFF;
                            }
                            else
                            {
                                // Si estaba fuera de rango, se realinea a múltiplo de 4:
                                if ((OAMADDR & 0x03) == 0x03)
                                    realignedBase = (OAMADDR + 1) & 0xFC;
                                else
                                    realignedBase = (OAMADDR + 4) & 0xFC;
                            }

                            oamOffset = (realignedBase + (spriteEvaluationIndex - 1) * 4) & 0xFF;
                        }
                    }

                    evaluateSingleSprite(oamOffset, scanline + 1);
                    ++spriteEvaluationIndex;
                }
            }

            // Ciclos 257 a 320: Lectura anticipada de patrones para la SIGUIENTE scanline (scanline + 1)
            if (renderScanLine && cycle >= 257 && cycle <= 320)
            {
                OAMADDR = 0;
                int fetchStep = (cycle - 257) % 8;
                int spriteIndex = (cycle - 257) / 8;
                int targetScanline = scanline + 1;

                if (spriteIndex < nextScanlineSpriteCount)
                {
                    const auto& sprite = nextScanlineSprites[spriteIndex];
                    const bool is8x16 = (PPUCTRL & 0x20) != 0;
                    const uint8_t spriteHeight = is8x16 ? 16 : 8;

                    int row = sprite.getRowInSprite(static_cast<uint8_t>(targetScanline), spriteHeight);

                    if (row >= 0 && row < spriteHeight)
                    {
                        uint16_t patternBase = 0;
                        uint8_t tileIndex = sprite.tileIndex;
                        uint8_t tileRow = static_cast<uint8_t>(row);

                        if (is8x16)
                        {
                            patternBase = (tileIndex & 0x01) ? 0x1000 : 0x0000;
                            tileIndex &= 0xFE;
                            if (tileRow >= 8)
                            {
                                ++tileIndex;
                                tileRow -= 8;
                            }
                        }
                        else
                        {
                            patternBase = (PPUCTRL & 0x08) ? 0x1000 : 0x0000;
                        }

                        const uint16_t tileAddress = patternBase + static_cast<uint16_t>(tileIndex) * 16;

                        if (fetchStep == 5)
                        {
                            const uint16_t lowAddress = tileAddress + tileRow;
                            nextSpritePatternLowBytes[spriteIndex] = readVRAM(lowAddress);
                        }
                        else if (fetchStep == 7)
                        {
                            const uint16_t highAddress = tileAddress + tileRow + 8;
                            nextSpritePatternHighBytes[spriteIndex] = readVRAM(highAddress);
                        }
                    }
                    else
                    {
                        nextSpritePatternLowBytes[spriteIndex] = 0;
                        nextSpritePatternHighBytes[spriteIndex] = 0;
                    }
                }
                else
                {
                    if (spriteIndex < 8)
                    {
                        nextSpritePatternLowBytes[spriteIndex] = 0;
                        nextSpritePatternHighBytes[spriteIndex] = 0;

                        // En hardware real, la PPU realiza fetches dummy del tile $FF
                        // manteniendo activa la línea A12 correspondiente durante los ciclos 257-320:
                        if (fetchStep == 5 || fetchStep == 7)
                        {
                            const bool is8x16 = (PPUCTRL & 0x20) != 0;
                            uint16_t dummyBase = is8x16 ? 0x1000 : ((PPUCTRL & 0x08) ? 0x1000 : 0x0000);
                            if (chrBridge.hasCartridge()) {
                                chrBridge.notifyAddress(dummyBase | 0x0FF0);
                            }
                        }
                    }
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

        if(visibleScanLine && cycle >= 1 && cycle <= 256 && !renderingEnabled)
        {
            // Si el renderizado está apagado, la pantalla muestra el color de fondo universal ($3F00)
            int x = cycle - 1;
            int y = scanline;
            uint16_t paletteAddr = ((v & 0x3F00) == 0x3F00) ? (0x3F00 | (v & 0x001F)) : 0x3F00;
            uint8_t colorIndex = readVRAM(paletteAddr) & 0x3F;

            if(PPUMASK & 0x01) colorIndex &= 0x30; // Escala de grises si está activa
            
            const auto& color = nesPalette[colorIndex];
            const auto emphasizedColor = applyColorEmphasis(color);

            frameBuffer.setPixel(x, y, emphasizedColor[0], emphasizedColor[1], emphasizedColor[2]);
        }

        uint16_t cyclesPerScanline = 341;

        if(scanline == -1 && renderingEnabled && oddFrame)
        {
            cyclesPerScanline = 340;
        }

        if(cycle >= cyclesPerScanline)
        {
            cycle = 0;
            ++scanline;

            if(scanline > 260)
            {
                scanline = -1;
                oddFrame = !oddFrame;

                frameComplete = true;
            }
        }
    }

    uint8_t PPU2C02::readVRAM(uint16_t addr)
    {
        uint16_t maskedAddr = addr & 0x3FFF;
        if (chrBridge.hasCartridge()) {
            ciram.setMirroring(chrBridge.getActiveMirroring());
            if (maskedAddr < 0x3F00) {
                chrBridge.notifyAddress(maskedAddr);
            }
        }
        return bus.read(maskedAddr);
    }

    void PPU2C02::writeVRAM(uint16_t addr, uint8_t value)
    {
        uint16_t maskedAddr = addr & 0x3FFF;
        if (chrBridge.hasCartridge()) {
            ciram.setMirroring(chrBridge.getActiveMirroring());
            if (maskedAddr < 0x3F00) {
                chrBridge.notifyAddress(maskedAddr);
            }
        }
        bus.write(maskedAddr, value);
    }

    uint8_t PPU2C02::readRegister(uint16_t addr)
    {
        uint8_t value = ioLatch;
        bool refreshDecay = false;

        switch(addr & 0x0007)
        {
            // PPUSTATUS
            case 0x0002:
                value = readPPUSTATUS();

                value = (value & 0xE0) | (ioLatch & 0x1F);
                refreshDecay = true;
                break;

            // OAMDATA
            case 0x0004:
                value = readOAMDATA();
                refreshDecay = true;
                break;

            // PPUDATA
            case 0x0007:
            {
                uint16_t currentVramAddr = v & 0x3FFF;
                value = readPPUDATA();

                if(currentVramAddr >= 0x3F00)
                {
                    uint8_t paletteData = value & 0x3F;
                    if(PPUMASK & 0x01)
                    {
                        paletteData &= 0x30;
                    }
                    value = paletteData | (ioLatch & 0xC0);
                }
                refreshDecay = true;
                break;
            }

            default:
                break;
        }


        ioLatch = value;
        if(refreshDecay) openBusTimer = 5369318;
        return value;
    }

    void PPU2C02::writeRegister(uint16_t addr, uint8_t value)
    {
        ioLatch = value;
        openBusTimer = 5369318;

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
        uint8_t oldCtrl = PPUCTRL;
        PPUCTRL = value;

        const bool nmiActive = (PPUSTATUS & 0x80) && (value & 0x80);
        if (nmiActive)
        {
            if (!(oldCtrl & 0x80))
            {
                nmiDelay = 2; // Retardo coherente de 1 dot
            }
        }
        else
        {
            nmiDelay = 0;
            mainBus->setNMILine(true);
        }

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

        if (scanline == 241)
        {
            if (cycle == 0)
            {
                suppressVBlankFlag = true;
                nmiDelay = 0;
                mainBus->setNMILine(true);
                mainBus->finishNMI();
            }
            else if (cycle == 1 || cycle == 2)
            {
                // En ciclos 1 y 2 la bandera se lee como 1 pero la NMI se suprime
                nmiDelay = 0;
                mainBus->setNMILine(true);
                mainBus->finishNMI();
            }
        }
        else if (value & 0x80)
        {
            // Fuera de la ventana de supresión, solo se libera la línea física
            // sin borrar el flanco ya capturado por la CPU
            mainBus->setNMILine(true);
        }

        PPUSTATUS &= 0x7F;
        writeToggle = false;
        return value;
    }

    // $2003 -> OAMADDR

    void PPU2C02::writeOAMADDR(uint8_t value)
    {
        OAMADDR = value;

        const bool renderingEnabled = (PPUMASK & 0x18) != 0;
        if (renderingEnabled && scanline >= 0 && scanline < 240)
        {
            uint8_t targetRow = value & 0xF8;
            for (int i = 0; i < 8; ++i)
            {
                OAM[targetRow + i] = OAM[i];
            }
        }
    }

    // $2004 -> OAMDATA

    uint8_t PPU2C02::readOAMDATA()
    {
        uint8_t value = OAM[OAMADDR];

        if((OAMADDR & 0x03) == 0x02)
        {
            value = (value & 0xE3);
        }
        return value;
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
            if (chrBridge.hasCartridge() && (v & 0x3FFF) < 0x3F00) {
                chrBridge.notifyAddress(v & 0x3FFF);
            }

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
            OAM[offset + 3],
            false
        };
    }

    Bus* PPU2C02::getMainBus() const {return mainBus;}

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
        openBusTimer = 0;

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
        oddFrame = false;

        oamM = 0;

        frameComplete = 0;

        suppressVBlankFlag = false;
        nmiDelay = 0;

        currentScanlineSpriteCount = 0;
        nextScanlineSpriteCount = 0;
        spritePatternLowBytes.fill(0);
        spritePatternHighBytes.fill(0);
        nextSpritePatternLowBytes.fill(0);
        nextSpritePatternHighBytes.fill(0);
    }

    bool PPU2C02::isFrameComplete() const {return frameComplete;}
    void PPU2C02::clearFrameComplete() {frameComplete = false;}

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
        bgPatternShiftLow = (bgPatternShiftLow << 1) | 0x0001;
        bgPatternShiftHigh = (bgPatternShiftHigh << 1) | 0x0001;

        bgAttributeShiftLow = (bgAttributeShiftLow << 1) | 0x0001;
        bgAttributeShiftHigh = (bgAttributeShiftHigh << 1) | 0x0001;
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

        // Obtener píxel de fondo
        NES::PPU::BackgroundPixel bgPixel = getBackgroundPixel();
        if (!bgVisible) bgPixel.transparent = true;

        // Obtener píxel de sprite prioritario
        EvaluatedSpritePixel spritePixel = getTopSpritePixel(x, y);
        if (!spriteVisible) spritePixel.transparent = true;

        // Detección de Sprite 0 Hit
        if (spritePixel.isSpriteZero && !spritePixel.transparent && !bgPixel.transparent && showBackground && showSprites)
        {
            if (x < 255 && x >= 0 && y < 240)
            {
                PPUSTATUS |= 0x40; // Activar bit de Sprite 0 Hit
            }
        }

        uint8_t finalColorIndex = 0;

        // Multiplexación de prioridades entre Fondo y Sprites
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

        // Aplicar escala de grises y énfasis de color
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