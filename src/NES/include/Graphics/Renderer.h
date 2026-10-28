#pragma once

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include <Graphics/FrameBuffer.h>

namespace TT::Graphics
{
    class Renderer
    {
        public:
            Renderer() = default;
            ~Renderer() = default;

            bool initialize(SDL_Window* window);

            void renderFrame(const NES::Graphics::FrameBuffer& frameBuffer);

            void destroy();

        private:
            bool createShaderProgram();

            GLuint compileShader(GLenum type, const char* source);

            SDL_Window* window = nullptr;

            GLuint shaderProgram = 0;
            GLuint texture = 0;
            GLuint vao = 0;
            GLuint vbo = 0;

            bool initialized = false;
        };
}