
#include <Graphics/Renderer.h>

#include <algorithm>
#include <iostream>

namespace TT::Graphics
{
    bool Renderer::initialize(SDL_Window* window)
    {
        if(window == nullptr)
        {
            std::cerr << "Renderer: ventana SDL invalida\n";
            return false;
        }

        this->window = window;

        if(!createShaderProgram())
        {
            destroy();
            return false;
        }

        // Textura de 256 x 240 píxeles en formato RGBA.
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_NEAREST
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_NEAREST
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE
        );

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            NES::Graphics::FrameBuffer::Width,
            NES::Graphics::FrameBuffer::Height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );

        // Coordenadas de textura invertidas verticalmente:
        // el primer píxel del framebuffer corresponde a la fila superior.
        const float vertices[] =
        {
            // Posición       // Coordenadas UV
            -1.0f,  1.0f,     0.0f, 0.0f,
            -1.0f, -1.0f,     0.0f, 1.0f,
             1.0f, -1.0f,     1.0f, 1.0f,

            -1.0f,  1.0f,     0.0f, 0.0f,
             1.0f, -1.0f,     1.0f, 1.0f,
             1.0f,  1.0f,     1.0f, 0.0f
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(vertices),
            vertices,
            GL_STATIC_DRAW
        );

        // Posición: 2 floats.
        glVertexAttribPointer(
            0,
            2,
            GL_FLOAT,
            GL_FALSE,
            4 * sizeof(float),
            reinterpret_cast<void*>(0)
        );
        glEnableVertexAttribArray(0);

        // Coordenadas UV: 2 floats.
        glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            4 * sizeof(float),
            reinterpret_cast<void*>(2 * sizeof(float))
        );
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        initialized = true;
        return true;
    }

    bool Renderer::createShaderProgram()
    {
        const char* vertexSource = R"(
            #version 330 core

            layout (location = 0) in vec2 aPosition;
            layout (location = 1) in vec2 aTexCoord;

            out vec2 texCoord;

            void main()
            {
                gl_Position = vec4(aPosition, 0.0, 1.0);
                texCoord = aTexCoord;
            }
        )";

        const char* fragmentSource = R"(
            #version 330 core

            in vec2 texCoord;

            out vec4 FragColor;

            uniform sampler2D screenTexture;

            void main()
            {
                FragColor = texture(screenTexture, texCoord);
            }
        )";

        GLuint vertexShader = compileShader(
            GL_VERTEX_SHADER,
            vertexSource
        );

        if(vertexShader == 0)
            return false;

        GLuint fragmentShader = compileShader(
            GL_FRAGMENT_SHADER,
            fragmentSource
        );

        if(fragmentShader == 0)
        {
            glDeleteShader(vertexShader);
            return false;
        }

        shaderProgram = glCreateProgram();

        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        GLint success = GL_FALSE;

        glGetProgramiv(
            shaderProgram,
            GL_LINK_STATUS,
            &success
        );

        if(success != GL_TRUE)
        {
            GLint logLength = 0;
            glGetProgramiv(
                shaderProgram,
                GL_INFO_LOG_LENGTH,
                &logLength
            );

            if(logLength > 0)
            {
                char* log = new char[logLength];

                glGetProgramInfoLog(
                    shaderProgram,
                    logLength,
                    nullptr,
                    log
                );

                std::cerr
                    << "Error al enlazar shaders:\n"
                    << log << '\n';

                delete[] log;
            }

            glDeleteProgram(shaderProgram);
            shaderProgram = 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        if(shaderProgram == 0)
            return false;

        glUseProgram(shaderProgram);

        glUniform1i(
            glGetUniformLocation(shaderProgram, "screenTexture"),
            0
        );

        glUseProgram(0);

        return true;
    }

    GLuint Renderer::compileShader(
        GLenum type,
        const char* source
    )
    {
        GLuint shader = glCreateShader(type);

        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success = GL_FALSE;

        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if(success != GL_TRUE)
        {
            GLint logLength = 0;

            glGetShaderiv(
                shader,
                GL_INFO_LOG_LENGTH,
                &logLength
            );

            if(logLength > 0)
            {
                char* log = new char[logLength];

                glGetShaderInfoLog(
                    shader,
                    logLength,
                    nullptr,
                    log
                );

                std::cerr
                    << "Error al compilar shader:\n"
                    << log << '\n';

                delete[] log;
            }

            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    void Renderer::renderFrame(
        const NES::Graphics::FrameBuffer& frameBuffer
    )
    {
        if(!initialized || window == nullptr)
            return;

        int windowWidth = 0;
        int windowHeight = 0;

        if(!SDL_GetWindowSizeInPixels(
            window,
            &windowWidth,
            &windowHeight
        ))
        {
            return;
        }

        if(windowWidth <= 0 || windowHeight <= 0)
            return;

        // Mantener la proporción 256:240 sin deformar la imagen.
        constexpr float aspectRatio =
            static_cast<float>(NES::Graphics::FrameBuffer::Width) /
            NES::Graphics::FrameBuffer::Height;

        int viewportWidth = windowWidth;
        int viewportHeight = static_cast<int>(
            viewportWidth / aspectRatio
        );

        if(viewportHeight > windowHeight)
        {
            viewportHeight = windowHeight;
            viewportWidth = static_cast<int>(
                viewportHeight * aspectRatio
            );
        }

        const int viewportX = (windowWidth - viewportWidth) / 2;
        const int viewportY = (windowHeight - viewportHeight) / 2;

        glViewport(0, 0, windowWidth, windowHeight);

        // Fondo negro alrededor de la imagen.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glViewport(
            viewportX,
            viewportY,
            viewportWidth,
            viewportHeight
        );

        // Actualizar la textura con los píxeles del framebuffer.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            NES::Graphics::FrameBuffer::Width,
            NES::Graphics::FrameBuffer::Height,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            frameBuffer.getPixels().data()
        );

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);

        // No se intercambian los buffers aquí.
        // El bucle principal debe llamar SDL_GL_SwapWindow().
    }

    void Renderer::destroy()
    {
        if(vbo != 0)
        {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }

        if(vao != 0)
        {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }

        if(texture != 0)
        {
            glDeleteTextures(1, &texture);
            texture = 0;
        }

        if(shaderProgram != 0)
        {
            glDeleteProgram(shaderProgram);
            shaderProgram = 0;
        }

        initialized = false;
        window = nullptr;
    }
}
