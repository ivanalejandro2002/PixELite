
#include <iostream>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include <NES/NES.h>
#include <Graphics/Renderer.h>
#include <cassert>

#include <chrono>
#include <thread>

#include <NES/PPU/PPUTests.h>

// Variables globales o dentro de tu clase de la aplicación/emulador
SDL_AudioStream* audioStream = nullptr;

// Constantes de temporización
constexpr double CPU_CLOCK_RATE = 1789773.5; // Frecuencia NTSC exacta
constexpr double SAMPLE_RATE = 44100.0;
constexpr double CYCLES_PER_SAMPLE = CPU_CLOCK_RATE / SAMPLE_RATE;
constexpr double TARGET_FPS = 60.098;
const auto frameDuration = std::chrono::duration<double>(1.0 / TARGET_FPS);
using Clock = std::chrono::steady_clock;

double sampleAccumulator = 0.0;
NES::AudioRingBuffer audioRingBuffer(44100);

bool initAudio()
{
    SDL_AudioSpec spec{};

    spec.freq = SAMPLE_RATE;
    spec.format = SDL_AUDIO_F32;
    spec.channels = 1;

    audioStream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &spec,
        nullptr,
        nullptr
    );

    if (!audioStream)
    {
        SDL_Log(
            "Fallo al abrir el stream de audio: %s",
            SDL_GetError()
        );
        return false;
    }

    // Consultar el backend de audio activo.
    const char* driver = SDL_GetCurrentAudioDriver();

    SDL_Log(
        "Audio driver: %s",
        driver ? driver : "Unknown"
    );

    // Consultar el dispositivo asociado al stream.
    SDL_AudioDeviceID device =
        SDL_GetAudioStreamDevice(audioStream);

    SDL_Log(
        "Audio device ID: %u",
        static_cast<unsigned int>(device)
    );

    // Consultar el formato real del dispositivo.
    SDL_AudioSpec deviceSpec{};

    if (SDL_GetAudioDeviceFormat(device, &deviceSpec, nullptr))
    {
        SDL_Log(
            "Device format: %d Hz, %d channels, format: %d",
            deviceSpec.freq,
            deviceSpec.channels,
            deviceSpec.format
        );
    }
    else
    {
        SDL_Log(
            "Could not get device format: %s",
            SDL_GetError()
        );
    }

    // Iniciar la reproducción.
    SDL_ResumeAudioStreamDevice(audioStream);

    return true;
}

void runEmulationFrame(
    NES::NES& nes,
    NES::APU& apu,
    NES::AudioRingBuffer& ringBuffer
)
{
    static float lowPassOutput = 0.0f;
    constexpr float ALPHA = 0.4f;

    while (!nes.getPPU().isFrameComplete())
    {
        nes.clock();

        // Un ciclo de CPU por cada tres ciclos del PPU.
        sampleAccumulator += 1.0 / 3.0;

        if (sampleAccumulator >= CYCLES_PER_SAMPLE)
        {
            sampleAccumulator -= CYCLES_PER_SAMPLE;

            const float rawSample = apu.testMode
                ? apu.getTestToneSample()
                : apu.getAudioSample();

            lowPassOutput += ALPHA * (rawSample - lowPassOutput);

            ringBuffer.push(lowPassOutput);
        }
    }

    nes.getPPU().clearFrameComplete();
}

void closeAudio()
{
    if (audioStream)
    {
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void triggerTestPulseChannel(NES::NES& nes)
{
    // Habilitar el canal de pulso 1 en el registro de control de la APU ($4015)
    nes.cpuWrite(0x4015, 0x01); // Bit 0 = Pulse 1 ENABLE

    // Configurar Pulse 1 ($4000 - $4003)
    nes.cpuWrite(0x4000, 0x9F); // Ciclo de trabajo al 50%, volumen constante al máximo
    nes.cpuWrite(0x4001, 0x00); // Sin sweep
    
    // Frecuencia (La de 440 Hz con timer = 0x00FD)
    nes.cpuWrite(0x4002, 0xFD); // Timer low (bits 0-7)
    
    // Para obtener una duración de ~2 segundos (~128 frames de longitud):
    // El índice en la tabla que da 128 (o un valor cercano como 96 o 192) se coloca en los bits 3-7 de $4003.
    // Usando el índice binario correspondiente (por ejemplo, 0x1A << 3 = 0xD0 o probando con un índice que active una duración alta).
    // Nota: Si colocas 0x00 en $4003, el índice es 0, lo que da una duración muy corta (10 frames / ~0.16 segundos).
    
    // Probemos cargando un índice de duración larga en $4003 (ej. índice 18 que da 96 frames, o índice 24 que da 192 frames):
    uint8_t lengthIndex = 10; // Esto selecciona un valor alto en la LENGTH_TABLE
    nes.cpuWrite(0x4003, (lengthIndex << 3)); // Carga el índice en los bits 3-7 y reinicia el faseo/envolvente
}

int main()
{
    // Inicializar SDL3.
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
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

    initAudio();

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

    /*NES::Testing::runAllPPUTests(nes);


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
        // 11 10 01 00
    }

    for(int i = 0x03C0; i < 0x0400; ++i)
    {
        nes.getPPU().writeVRAM(0x2800+i, 0xE4);
    }

    nes.getPPU().writeVRAM(0x2800 + 0x03C0 + 0x0016, 0x1B);

    // nes.getPPU().debugGenerateBackground();
    NES::Testing::fillNametable(nes.getPPU(), 0x2000, 3);
    NES::Testing::fillNametable(nes.getPPU(), 0x2800, 3);

    
    nes.getPPU().writePPUSCROLL(10);
    nes.getPPU().writePPUSCROLL(100);

    //nes.getPPU().debugGenerateBackground();

    nes.getPPU().writePPUMASK(0x1E);


    for (int i = 0; i < 341 * 262; ++i)
    {
        nes.getPPU().clock();
    }

    NES::Testing::fillNametable(nes.getPPU(), 0x2000, 2);

    std::cout << "\n\n\n";

    nes.getPPU().writeRegister(0x2003, 0x00);
    for(int i = 0; i < 256; ++i) {
        nes.getPPU().writeRegister(0x2004, 0xFF); // Todos los Y en 0xFF (invisibles)
    }

    nes.getPPU().writeRegister(0x2003, 0x00); 
    nes.getPPU().writeRegister(0x2004, 0xFF);   // Y = 16 (en pantalla será la línea 51)
    nes.getPPU().writeRegister(0x2004, 0x03); // Tile Index = 1
    nes.getPPU().writeRegister(0x2004, 0x40); // Atributos (Paleta 0, al frente)
    nes.getPPU().writeRegister(0x2004, 0x3A);   // X = 60

    nes.getPPU().writeRegister(0x2003, 0x04); 
    nes.getPPU().writeRegister(0x2004, 80);   // Y
    nes.getPPU().writeRegister(0x2004, 0x02); // Tile
    nes.getPPU().writeRegister(0x2004, 0x00); // Atributos
    nes.getPPU().writeRegister(0x2004, 90);   // X

    NES::Testing::fillNametable(nes.getPPU(), 0x2800, 3);

    for (int i = 0; i < 341 * 262; ++i)
    {
        nes.getPPU().clock();
    }

    nes.memoryDebug();
    */

    // Definimos un tamaño de búfer ideal (ej. el equivalente a ~3 fotogramas de audio)
    // 44100 Hz / 60.098 FPS = ~734 muestras por frame. 
    // 3 frames * 734 muestras * 4 bytes (float) = ~8808 bytes.
    bool running = true;
    
    const size_t TARGET_BUFFER_BYTES = 12000;

    nes.getAPU().deactivateTestMode();

    if (audioStream == nullptr)
    {
        SDL_Log("¡CRÍTICO: audioStream es nullptr antes de arrancar el bucle!");
        return -1; // O abortar
    }


    // AccuracyTesting CPU:
    // nes.loadROM("../NESROMS/accuracy testings/branch_timing_tests/1.Branch_Basics.nes");
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/instr_test-v5/official_only.nes");
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/instr_test-v5/all_instrs.nes");
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/instr_timing/instr_timing.nes");
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/cpu_timing_test6/cpu_timing_test.nes"); X

    // AccuracyTesting PPU
    // nes.loadROM("../NESROMS/accuracy testings/PPU/palette.nes");
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/blargg_ppu_tests_2005.09.15b/palette_ram.nes");
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/blargg_ppu_tests_2005.09.15b/power_up_palette.nes");
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/blargg_ppu_tests_2005.09.15b/sprite_ram.nes");
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/blargg_ppu_tests_2005.09.15b/vbl_clear_time.nes");
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/blargg_ppu_tests_2005.09.15b/vram_access.nes");
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/full_nes_palette.nes");
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/nmi_sync/demo_ntsc.nes");
    // nes.loadROM("../NESROMS/accuracy testings/ntsc_torture.nes"); 
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/ppu_open_bus/ppu_open_bus.nes");
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/sprite_hit_tests_2005.10.05/02.alignment.nes");

    // APU
    // nes.loadROM("../NESROMS/accuracy testings/nes-test-roms-master/apu_test/apu_test.nes");


    // nes.loadROM("../NESROMS/oficial/BestNESGames/DuckTales (USA).nes");
    // nes.loadROM("../NESROMS/oficial/BestNESGames/Dig Dug (Japan).nes");
    // nes.loadROM("../NESROMS/oficial/BestNESGames/Zelda/Legend of Zelda, The (USA) (Rev 1).nes");
    // nes.loadROM("../NESROMS/oficial/BestNESGames/Metroid (USA).nes");
    nes.loadROM("../NESROMS/oficial/BestNESGames/Mario/Super Mario Bros. (World).nes");
    // nes.loadROM("../NESROMS/oficial/BestNESGames/Lemmings (USA).nes");

    // nes.loadROM("../NESROMS/homebrew/All Hell Unleashed v6.66.nes");
    // nes.loadROM("../NESROMS/homebrew/Banana Bash! Demo.nes");
    // nes.loadROM("../NESROMS/homebrew/from_below_2020_09_16_v_1_0_0.nes");
    // nes.loadROM("../NESROMS/homebrew/Penguin Sokoban Adventure v1.1.nes");
    // nes.loadROM("../NESROMS/homebrew/Spacegulls-1.1.nes");
    // nes.loadROM("../NESROMS/homebrew/Super Bat Puncher Demo.nes");

    // nes.loadROM("../NESROMS/oficial/BestNESGames/Mega Man/Mega Man (USA).nes");
    // nes.loadROM("../NESROMS/AC/AccuracyCoin-main/AccuracyCoin.nes");


    for (int i = 0; i < 8; ++i) {
        runEmulationFrame(nes, nes.getAPU(), audioRingBuffer);
    }

    const size_t targetBufferSize = 2048; 
    double drcRatio = 1.0;
    double srcPosition = 0.0;

    constexpr int samplesPerFrame = 734;
    std::vector<float> fixedFrameSamples(samplesPerFrame);

    auto nextFrameTime = Clock::now();

    while(running)
    {
        SDL_Event event;

        // --------------------------------------------------
        // 1. Procesamiento de eventos
        // --------------------------------------------------

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN ||
                event.type == SDL_EVENT_KEY_UP)
            {
                bool isPressed = (event.type == SDL_EVENT_KEY_DOWN);
                SDL_Keycode key = event.key.key;

                // Mapeo clásico de teclado para la NES.
                if (key == SDLK_Z)
                    nes.getController1().setButtonState(
                        NES::Controller::A, isPressed);

                if (key == SDLK_X)
                    nes.getController1().setButtonState(
                        NES::Controller::B, isPressed);

                if (key == SDLK_RSHIFT)
                    nes.getController1().setButtonState(
                        NES::Controller::SELECT, isPressed);

                if (key == SDLK_RETURN)
                    nes.getController1().setButtonState(
                        NES::Controller::START, isPressed);

                if (key == SDLK_UP)
                    nes.getController1().setButtonState(
                        NES::Controller::UP, isPressed);

                if (key == SDLK_DOWN)
                    nes.getController1().setButtonState(
                        NES::Controller::DOWN, isPressed);

                if (key == SDLK_LEFT)
                    nes.getController1().setButtonState(
                        NES::Controller::LEFT, isPressed);

                if (key == SDLK_RIGHT)
                    nes.getController1().setButtonState(
                        NES::Controller::RIGHT, isPressed);
            }
        }

        if (!running)
            break;


        // --------------------------------------------------
        // 2. Ejecutar un frame de emulación
        // --------------------------------------------------

        auto emulationStart = Clock::now();

        runEmulationFrame(nes, nes.getAPU(), audioRingBuffer);

        auto emulationEnd = Clock::now();

        double emulationMs =
            std::chrono::duration<double, std::milli>(
                emulationEnd - emulationStart
            ).count();


        // --------------------------------------------------
        // 3. Obtener el estado de los buffers
        // --------------------------------------------------

        constexpr double TARGET_TOTAL_SAMPLES = 2205.0;

        constexpr double DRC_GAIN = 0.00001;
        constexpr double DRC_SMOOTHING = 0.05;

        size_t ringAvailable = audioRingBuffer.available();

        int queuedBytes = SDL_GetAudioStreamQueued(audioStream);

        size_t sdlSamples = queuedBytes > 0
            ? static_cast<size_t>(queuedBytes) / sizeof(float)
            : 0;

        double totalSamples =
            static_cast<double>(ringAvailable + sdlSamples);

        double totalLatencyMs =
            1000.0 * totalSamples / SAMPLE_RATE;


        // --------------------------------------------------
        // 4. Dynamic Rate Control
        // --------------------------------------------------

        double error = totalSamples - TARGET_TOTAL_SAMPLES;

        double desiredRatio = std::clamp(
            1.0 + error * DRC_GAIN,
            0.995,
            1.005
        );

        drcRatio +=
            (desiredRatio - drcRatio) * DRC_SMOOTHING;


        // --------------------------------------------------
        // 5. Extraer muestras con resampleo DRC
        // --------------------------------------------------

        size_t extracted = audioRingBuffer.readResampled(
            fixedFrameSamples.data(),
            samplesPerFrame,
            drcRatio,
            srcPosition
        );


        // --------------------------------------------------
        // 6. Enviar audio a SDL3
        // --------------------------------------------------

        bool audioSubmitted = true;

        if (extracted > 0)
        {
            audioSubmitted = SDL_PutAudioStreamData(
                audioStream,
                fixedFrameSamples.data(),
                static_cast<int>(extracted * sizeof(float))
            );

            if (!audioSubmitted)
            {
                std::cerr
                    << "Error al enviar audio a SDL: "
                    << SDL_GetError()
                    << '\n';
            }
        }


        // --------------------------------------------------
        // 7. Medir audio pendiente después del envío
        // --------------------------------------------------

        int queuedBytesAfter = SDL_GetAudioStreamQueued(audioStream);

        size_t sdlSamplesAfter = queuedBytesAfter > 0
            ? static_cast<size_t>(queuedBytesAfter) / sizeof(float)
            : 0;

        size_t ringAfter = audioRingBuffer.available();

        double totalSamplesAfter =
            static_cast<double>(ringAfter + sdlSamplesAfter);

        double latencyAfterMs =
            1000.0 * totalSamplesAfter / SAMPLE_RATE;


        // --------------------------------------------------
        // 8. Diagnóstico
        // --------------------------------------------------

        static int debugFrames = 0;

        if (++debugFrames >= 60)
        {
            debugFrames = 0;

            // std::cout
            //     << "Emulation: " << emulationMs << " ms"
            //     << " | Ring: " << ringAfter << " samples"
            //     << " | Extracted: " << extracted << " samples"
            //     << " | SDL: "
            //     << (1000.0 * sdlSamplesAfter / SAMPLE_RATE) << " ms"
            //     << " | Total latency: " << latencyAfterMs << " ms"
            //     << " | DRC: " << drcRatio
            //     << " | Audio submitted: "
            //     << (audioSubmitted ? "OK" : "FAILED")
            //     << '\n';
        }


        // --------------------------------------------------
        // 9. Renderizado y sincronización gráfica
        // --------------------------------------------------

        renderer.renderFrame(nes.getPPU().getFramebuffer());

        SDL_GL_SwapWindow(window);


        // --------------------------------------------------
        // 10. Limitador de velocidad
        // --------------------------------------------------

        auto now = Clock::now();

        // Evitar acumular retrasos grandes.
        if (now > nextFrameTime + std::chrono::milliseconds(100))
        {
            nextFrameTime = now;
        }

        nextFrameTime +=
            std::chrono::duration_cast<Clock::duration>(
                std::chrono::duration<double, std::milli>(
                    1000.0 / 60.098
                )
            );

        if (nextFrameTime > Clock::now())
        {
            std::this_thread::sleep_until(nextFrameTime);
        }
    }
    closeAudio();
    nes.memoryDebug();

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
