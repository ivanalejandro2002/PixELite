#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

namespace NES::Graphics {

    class FrameBuffer {
        public:
            static constexpr int Width = 256;
            static constexpr int Height = 240;
            static constexpr int BytesPerPixel = 4;

            using PixelData = std::array<
                uint8_t,
                Width * Height * BytesPerPixel
            >;

            FrameBuffer();

            void clear(
                uint8_t r = 0,
                uint8_t g = 0,
                uint8_t b = 0,
                uint8_t a = 255
            );

            void setPixel(
                int x,
                int y,
                uint8_t r,
                uint8_t g,
                uint8_t b,
                uint8_t a = 255
            );

            const PixelData& getPixels() const;

        private:
            PixelData pixels{};
        };

}