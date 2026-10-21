
#include <iostream>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include <NES/NES.h>
#include <Graphics/Renderer.h>

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

    // Inicializar emulador.
    NES::NES nes;

    // Inicializar renderer.
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

    nes.getPPU().writeRegister(0x2005, 10);
    nes.getPPU().writeRegister(0x2005, 0);

    // Generar patrón gráfico de prueba.
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
    for(int i = 0; i <= 0x03BF; ++i)
    {
        nes.getPPU().writeVRAM(0x2000 + i, 0x02);
    }

    for(int i = 0x23C0; i < 0x2400; ++i)
    {
        nes.getPPU().writeVRAM(i, 0xE4);
    }
    nes.getPPU().debugGenerateBackground();

    

    bool running = true;

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
