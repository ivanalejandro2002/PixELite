#include <Graphics/FrameBuffer.h>

namespace NES::Graphics {

    FrameBuffer::FrameBuffer()
    {
        clear();
    }

    void FrameBuffer::clear(
        uint8_t r,
        uint8_t g,
        uint8_t b,
        uint8_t a
    ) {
        for (size_t i = 0; i < pixels.size(); i += BytesPerPixel) {
            pixels[i + 0] = r;
            pixels[i + 1] = g;
            pixels[i + 2] = b;
            pixels[i + 3] = a;
        }
    }

    void FrameBuffer::setPixel(
        int x,
        int y,
        uint8_t r,
        uint8_t g,
        uint8_t b,
        uint8_t a
    ) {
        if (x < 0 || x >= Width ||
            y < 0 || y >= Height) {
            return;
        }

        const size_t index =
            (static_cast<size_t>(y) * Width + x) * BytesPerPixel;

        pixels[index + 0] = r;
        pixels[index + 1] = g;
        pixels[index + 2] = b;
        pixels[index + 3] = a;
    }

    const FrameBuffer::PixelData& FrameBuffer::getPixels() const
    {
        return pixels;
    }

}