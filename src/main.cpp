#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <NES/NES.h>
#include <Graphics/Renderer.h>
#include <GUI/EmulatorGUI.h>

SDL_AudioStream* audioStream = nullptr;

constexpr double CPU_CLOCK_RATE = 1789773.5;
constexpr double SAMPLE_RATE = 44100.0;
constexpr double CYCLES_PER_SAMPLE = CPU_CLOCK_RATE / SAMPLE_RATE;
constexpr double TARGET_FPS = 60.098;
using Clock = std::chrono::steady_clock;

double sampleAccumulator = 0.0;
NES::AudioRingBuffer audioRingBuffer(44100);

bool initAudio()
{
    SDL_AudioSpec spec{};
    spec.freq = static_cast<int>(SAMPLE_RATE);
    spec.format = SDL_AUDIO_F32;
    spec.channels = 1;

    audioStream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &spec,
        nullptr,
        nullptr
    );

    if (!audioStream) {
        SDL_Log("Fallo al abrir el stream de audio: %s", SDL_GetError());
        return false;
    }

    SDL_ResumeAudioStreamDevice(audioStream);
    return true;
}

void closeAudio()
{
    if (audioStream) {
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void runEmulationFrame(NES::NES& nes, NES::APU& apu, NES::AudioRingBuffer& ringBuffer)
{
    // Filtros analógicos de salida de la NES a 44100 Hz:
    // 1) High-Pass Filter (~90 Hz) para eliminar el offset DC del DAC no lineal
    // 2) Low-Pass Filter (~14 kHz) para suavizar el aliasing de muestreo
    static float prevRawSample = 0.0f;
    static float highPassOutput = 0.0f;
    static float lowPassOutput = 0.0f;

    constexpr float HPF_ALPHA = 0.9873f; // ~90 Hz @ 44.1 kHz
    constexpr float LPF_ALPHA = 0.65f;   // ~14 kHz @ 44.1 kHz

    while (!nes.getPPU().isFrameComplete())
    {
        nes.clock();
        sampleAccumulator += 1.0 / 3.0;

        if (sampleAccumulator >= CYCLES_PER_SAMPLE)
        {
            sampleAccumulator -= CYCLES_PER_SAMPLE;
            const float rawSample = static_cast<float>(
                apu.testMode ? apu.getTestToneSample() : apu.getAudioSample()
            );

            // 1. Filtro Pasa-Altas (centra la onda en 0.0 y evita saturación DC)
            highPassOutput = HPF_ALPHA * (highPassOutput + rawSample - prevRawSample);
            prevRawSample = rawSample;

            // 2. Filtro Pasa-Bajas suave
            lowPassOutput += LPF_ALPHA * (highPassOutput - lowPassOutput);

            ringBuffer.push(lowPassOutput);
        }
    }
    nes.getPPU().clearFrameComplete();
}

// Genera las imágenes 128x128 de las dos Pattern Tables ($0000 y $1000) solo cuando la ventana está abierta
void extractPatternTables(NES::NES& nes, NES::GUI::DebugSnapshot& snap, int paletteOption)
{
    std::array<std::array<uint8_t, 3>, 4> activePalette = {{
        { 15,  15,  25},
        { 85,  95, 115},
        {165, 175, 195},
        {245, 245, 255}
    }};

    // Si paletteOption > 0, leemos la subpaleta real desde $3F00 - $3F1F en la PPU:
    // 1..4 -> Paletas de Fondo (0..3) | 5..8 -> Paletas de Sprite (4..7)
    if (paletteOption >= 1 && paletteOption <= 8) {
        uint16_t palIndex = static_cast<uint16_t>(paletteOption - 1);
        uint16_t basePalAddr = 0x3F00 + (palIndex * 4);

        // El color 0 siempre es el color de fondo universal en $3F00
        uint8_t universalBg = nes.getPPU().readVRAM(0x3F00) & 0x3F;
        activePalette[0] = nes.getPPU().getMasterPaletteRGB(universalBg);

        for (int c = 1; c < 4; ++c) {
            uint8_t nesColorIdx = nes.getPPU().readVRAM(basePalAddr + c) & 0x3F;
            activePalette[c] = nes.getPPU().getMasterPaletteRGB(nesColorIdx);
        }
    }

    snap.activePaletteColors = activePalette;

    auto decodeTable = [&](uint16_t baseAddr, std::array<uint8_t, 128 * 128 * 4>& outBuf) {
        for (int tileY = 0; tileY < 16; ++tileY) {
            for (int tileX = 0; tileX < 16; ++tileX) {
                uint16_t tileOffset = baseAddr + (tileY * 16 + tileX) * 16;
                for (int row = 0; row < 8; ++row) {
                    uint8_t lowPlane  = nes.getCartridge().ppuRead(tileOffset + row);
                    uint8_t highPlane = nes.getCartridge().ppuRead(tileOffset + row + 8);
                    for (int col = 0; col < 8; ++col) {
                        uint8_t bit = 7 - col;
                        uint8_t pixel = ((lowPlane >> bit) & 1) | (((highPlane >> bit) & 1) << 1);
                        int px = tileX * 8 + col;
                        int py = tileY * 8 + row;
                        size_t idx = (py * 128 + px) * 4;
                        outBuf[idx + 0] = activePalette[pixel][0];
                        outBuf[idx + 1] = activePalette[pixel][1];
                        outBuf[idx + 2] = activePalette[pixel][2];
                        outBuf[idx + 3] = 255;
                    }
                }
            }
        }
    };

    decodeTable(0x0000, snap.patternTable0);
    decodeTable(0x1000, snap.patternTable1);
    snap.patternTablesUpdated = true;
}

void updateDebugSnapshot(NES::NES& nes, NES::GUI::DebugSnapshot& snap, bool includeRam)
{
    snap.ppuCtrl   = nes.getPPU().debugPPUCTRL();
    snap.ppuMask   = nes.getPPU().debugPPUMASK();
    snap.ppuStatus = nes.getPPU().debugPPUSTATUS();
    snap.oamAddr   = nes.getPPU().debugOAMADDR();
    snap.v         = nes.getPPU().debugV();
    snap.t         = nes.getPPU().debugT();
    snap.fineX     = nes.getPPU().debugX();
    snap.w         = nes.getPPU().debugW();

    snap.a      = nes.getCPU().getA();
    snap.x      = nes.getCPU().getX();
    snap.y      = nes.getCPU().getY();
    snap.pc     = nes.getCPU().getPC();
    snap.s      = nes.getCPU().getSP();
    snap.status = nes.getCPU().getStatus().getByte();

    snap.mapperId  = nes.getCartridge().getMapperNumber();
    snap.prgSizeKB = nes.getCartridge().getPRG().size() / 1024;
    snap.chrSizeKB = nes.getCartridge().getCHR().size() / 1024;

    if (includeRam) {
        for (uint16_t addr = 0; addr < 0x0800; ++addr) {
            snap.cpuRam[addr] = nes.cpuRead(addr);
        }
    }
}

int main()
{
    // En Linux (GNOME/Wayland), forzar el backend X11 (XWayland) permite que
    // SDL3 e ImGui creen ventanas Multi-Viewport independientes fuera de la ventana principal:
    #ifdef __linux__
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11,wayland");
    #endif

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::cerr << "Error al inicializar SDL3: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* window = SDL_CreateWindow(
        "PixELite - NES",
        1024,
        960,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1); // VSync para el hilo de UI/Render

    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress))) {
        std::cerr << "Error al inicializar GLAD\n";
        return 1;
    }

    if (!initAudio()) {
        return -1;
    }

    // --------------------------------------------------
    // Inicializar Dear ImGui (con soporte Multi-Viewport)
    // --------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Permite acoplar ventanas
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Permite sacar ventanas fuera de la ventana principal

    ImGui::StyleColorsDark();

    // Cuando Viewports está activo, ajustamos WindowRounding y el alfa del fondo
    // para que las ventanas que salen al escritorio se vean como ventanas nativas:
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Cargar fuente TrueType a buen tamaño base (24px) para que al escalar dinámicamente se vea nítida
    const char* fontPaths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "C:/Windows/Fonts/consola.ttf"
    };
    bool fontLoaded = false;
    for (const char* path : fontPaths) {
        if (std::filesystem::exists(path)) {
            io.Fonts->AddFontFromFileTTF(path, 20.0f);
            fontLoaded = true;
            break;
        }
    }
    if (!fontLoaded) {
        ImFontConfig cfg;
        cfg.SizePixels = 18.0f;
        io.Fonts->AddFontDefault(&cfg);
    }
    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    TT::Graphics::Renderer renderer;
    if (!renderer.initialize(window)) {
        return 1;
    }

    NES::NES nes;
    nes.getAPU().deactivateTestMode();

    NES::GUI::EmulatorGUI gui;
    gui.initOpenGLResources();

    // Buffers compartidos entre el Hilo de Emulación y el Hilo de Render/UI
    std::mutex frameMutex;
    NES::Graphics::FrameBuffer sharedFrameBuffer;
    NES::GUI::DebugSnapshot sharedDebugSnapshot;
    std::atomic<uint8_t> atomicControllerState{0};
    std::atomic<bool> appRunning{true};

    // ==================================================
    // HILO 1: BUCLE DEDICADO DE EMULACIÓN + AUDIO DRC
    // ==================================================
    std::thread emulationThread([&]() {
        double drcRatio = 1.0;
        double srcPosition = 0.0;
        constexpr int samplesPerFrame = 734;
        std::vector<float> fixedFrameSamples(samplesPerFrame);

        auto nextFrameTime = Clock::now();
        auto fpsTimer = Clock::now();
        int framesCounted = 0;

        while (appRunning.load(std::memory_order_relaxed)) {
            // 1. Procesar comandos enviados desde la GUI
            if (gui.cmdLoadRom.exchange(false)) {
                std::string path;
                {
                    std::lock_guard<std::mutex> lock(gui.stateMutex);
                    path = gui.pendingRomPath;
                }
                if (!path.empty() && nes.loadROM(path)) {
                    nes.reset();
                    SDL_ClearAudioStream(audioStream);
                    gui.confirmRomLoaded(path);

                    // Limpiar y actualizar inmediatamente el FrameBuffer compartido y el DebugSnapshot
                    std::lock_guard<std::mutex> lock(frameMutex);
                    sharedFrameBuffer = nes.getPPU().getFramebuffer();
                    updateDebugSnapshot(nes, sharedDebugSnapshot, gui.showRAM);
                    if (gui.showPatternTables) {
                        extractPatternTables(nes, sharedDebugSnapshot, gui.selectedPatternPalette.load(std::memory_order_relaxed));
                    }
                    nextFrameTime = Clock::now();
                }
            }

            if (gui.cmdStart.exchange(false)) {
                std::string path;
                {
                    std::lock_guard<std::mutex> lock(gui.stateMutex);
                    path = gui.currentRomPath;
                    if (!path.empty()) gui.currentState = NES::GUI::EmulationState::Running;
                }
                if (!path.empty()) {
                    nes.loadROM(path);
                    nextFrameTime = Clock::now();
                }
            }

            if (gui.cmdStop.exchange(false)) {
                std::lock_guard<std::mutex> lock(gui.stateMutex);
                gui.currentState = NES::GUI::EmulationState::Stopped;
                gui.linkedSaveStatePath = "";
                SDL_ClearAudioStream(audioStream);
            }

            // --- GUARDAR SAVESTATE (.pxsave) ---
            // Al estar aquí al inicio del bucle del frame, el frame anterior ya terminó por completo
            // (la PPU y el CPU están en un límite seguro) y NO alteramos gui.currentState:
            // si estaba Running sigue Running; si estaba Paused sigue Paused.
            if (gui.cmdSaveState.exchange(false)) {
                std::string savePath;
                NES::GUI::EmulationState st;
                {
                    std::lock_guard<std::mutex> lock(gui.stateMutex);
                    savePath = gui.pendingSaveStatePath;
                    st = gui.currentState;
                }
                if (!savePath.empty() &&
                    (st == NES::GUI::EmulationState::Running || st == NES::GUI::EmulationState::Paused)) {
                    nes.saveStateToFile(savePath);
                }
            }

            // --- CARGAR SAVESTATE (.pxsave) ---
            if (gui.cmdLoadState.exchange(false)) {
                std::string loadPath;
                NES::GUI::EmulationState st;
                {
                    std::lock_guard<std::mutex> lock(gui.stateMutex);
                    loadPath = gui.pendingLoadStatePath;
                    st = gui.currentState;
                }
                if (!loadPath.empty() &&
                    (st == NES::GUI::EmulationState::Running || st == NES::GUI::EmulationState::Paused)) {
                    if (nes.loadStateFromFile(loadPath)) {
                        SDL_ClearAudioStream(audioStream);

                        // Actualizar el FrameBuffer de OpenGL y todas las ventanas de Debug al instante
                        // (incluso si el emulador está en Pausa):
                        std::lock_guard<std::mutex> lock(frameMutex);
                        sharedFrameBuffer = nes.getPPU().getFramebuffer();
                        updateDebugSnapshot(nes, sharedDebugSnapshot, gui.showRAM);
                        if (gui.showPatternTables) {
                            extractPatternTables(nes, sharedDebugSnapshot, gui.selectedPatternPalette.load(std::memory_order_relaxed));
                        }
                    }
                }
            }

            if (gui.cmdReset.exchange(false)) {
                nes.reset(); // O recargar el estado de inicio de la consola
            }

            if (gui.cmdTogglePause.exchange(false)) {
                std::lock_guard<std::mutex> lock(gui.stateMutex);
                if (gui.currentState == NES::GUI::EmulationState::Running)
                    gui.currentState = NES::GUI::EmulationState::Paused;
                else if (gui.currentState == NES::GUI::EmulationState::Paused)
                    gui.currentState = NES::GUI::EmulationState::Running;
            }

            // Verificar si el usuario solicitó avanzar exactamente 1 frame
            bool stepOneFrame = gui.cmdStepFrame.exchange(false);
            if (stepOneFrame) {
                std::lock_guard<std::mutex> lock(gui.stateMutex);
                if (gui.currentState == NES::GUI::EmulationState::Running) {
                    // Si estaba corriendo, lo pasamos a Pausa y avanzamos solo este frame
                    gui.currentState = NES::GUI::EmulationState::Paused;
                } else if (gui.currentState != NES::GUI::EmulationState::Paused) {
                    // Si no hay ROM activa, ignoramos el step
                    stepOneFrame = false;
                }
            }

            NES::GUI::EmulationState stateNow;
            {
                std::lock_guard<std::mutex> lock(gui.stateMutex);
                stateNow = gui.currentState;
            }

            // Si no está en ejecución NI se pidió avanzar un frame, dormimos brevemente
            if (stateNow != NES::GUI::EmulationState::Running && !stepOneFrame) {
                if (gui.showPatternTables) {
                    std::lock_guard<std::mutex> lock(frameMutex);
                    extractPatternTables(nes, sharedDebugSnapshot, gui.selectedPatternPalette.load(std::memory_order_relaxed));
                }
                gui.emulationFps = 0.0f;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                nextFrameTime = Clock::now();
                continue;
            }

            // 2. Actualizar botones del mando desde el estado atómico capturado en UI
            uint8_t buttons = atomicControllerState.load(std::memory_order_relaxed);
            nes.getController1().setButtonState(NES::Controller::A,      (buttons & (1 << 0)) != 0);
            nes.getController1().setButtonState(NES::Controller::B,      (buttons & (1 << 1)) != 0);
            nes.getController1().setButtonState(NES::Controller::SELECT, (buttons & (1 << 2)) != 0);
            nes.getController1().setButtonState(NES::Controller::START,  (buttons & (1 << 3)) != 0);
            nes.getController1().setButtonState(NES::Controller::UP,     (buttons & (1 << 4)) != 0);
            nes.getController1().setButtonState(NES::Controller::DOWN,   (buttons & (1 << 5)) != 0);
            nes.getController1().setButtonState(NES::Controller::LEFT,   (buttons & (1 << 6)) != 0);
            nes.getController1().setButtonState(NES::Controller::RIGHT,  (buttons & (1 << 7)) != 0);

            // 3. Ejecutar 1 frame completo de NES
            runEmulationFrame(nes, nes.getAPU(), audioRingBuffer);

            // 4. Dynamic Rate Control (DRC) y envío de Audio a SDL3
            constexpr double TARGET_TOTAL_SAMPLES = 2205.0;
            constexpr double DRC_GAIN = 0.00001;
            constexpr double DRC_SMOOTHING = 0.05;

            size_t ringAvailable = audioRingBuffer.available();
            int queuedBytes = SDL_GetAudioStreamQueued(audioStream);
            size_t sdlSamples = queuedBytes > 0 ? static_cast<size_t>(queuedBytes) / sizeof(float) : 0;
            double totalSamples = static_cast<double>(ringAvailable + sdlSamples);

            double error = totalSamples - TARGET_TOTAL_SAMPLES;
            double desiredRatio = std::clamp(1.0 + error * DRC_GAIN, 0.995, 1.005);
            drcRatio += (desiredRatio - drcRatio) * DRC_SMOOTHING;

            size_t extracted = audioRingBuffer.readResampled(
                fixedFrameSamples.data(),
                samplesPerFrame,
                drcRatio,
                srcPosition
            );

            if (extracted > 0) {
                SDL_PutAudioStreamData(
                    audioStream,
                    fixedFrameSamples.data(),
                    static_cast<int>(extracted * sizeof(float))
                );
            }

            // 5. Publicar FrameBuffer y Snapshot de Debug hacia el Hilo de Render
            {
                std::lock_guard<std::mutex> lock(frameMutex);
                sharedFrameBuffer = nes.getPPU().getFramebuffer();

                // Actualizar registros PPU
                sharedDebugSnapshot.ppuCtrl   = nes.getPPU().debugPPUCTRL();
                sharedDebugSnapshot.ppuMask   = nes.getPPU().debugPPUMASK();
                sharedDebugSnapshot.ppuStatus = nes.getPPU().debugPPUSTATUS();
                sharedDebugSnapshot.oamAddr   = nes.getPPU().debugOAMADDR();
                sharedDebugSnapshot.v         = nes.getPPU().debugV();
                sharedDebugSnapshot.t         = nes.getPPU().debugT();
                sharedDebugSnapshot.fineX     = nes.getPPU().debugX();
                sharedDebugSnapshot.w         = nes.getPPU().debugW();

                // Actualizar registros CPU (ajusta los nombres a los getters de tu clase CPU)
                sharedDebugSnapshot.a      = nes.getCPU().getA();
                sharedDebugSnapshot.x      = nes.getCPU().getX();
                sharedDebugSnapshot.y      = nes.getCPU().getY();
                sharedDebugSnapshot.pc     = nes.getCPU().getPC();
                sharedDebugSnapshot.s      = nes.getCPU().getSP();
                sharedDebugSnapshot.status = nes.getCPU().getStatus().getByte();

                sharedDebugSnapshot.mapperId  = nes.getCartridge().getMapperNumber();
                sharedDebugSnapshot.prgSizeKB = nes.getCartridge().getPRG().size() / 1024;
                sharedDebugSnapshot.chrSizeKB = nes.getCartridge().getCHR().size() / 1024;

                if (gui.showRAM) {
                    for (uint16_t addr = 0; addr < 0x0800; ++addr) {
                        sharedDebugSnapshot.cpuRam[addr] = nes.cpuRead(addr);
                    }
                }

                if (gui.showPatternTables) {
                    extractPatternTables(nes, sharedDebugSnapshot, gui.selectedPatternPalette.load(std::memory_order_relaxed));
                } else {
                    sharedDebugSnapshot.patternTablesUpdated = false;
                }
            }

            // 6. Medición de FPS reales de emulación
            framesCounted++;
            auto now = Clock::now();
            double elapsedSec = std::chrono::duration<double>(now - fpsTimer).count();
            if (elapsedSec >= 0.5) {
                gui.emulationFps = static_cast<float>(framesCounted / elapsedSec);
                framesCounted = 0;
                fpsTimer = now;
            }

            // 7. Sincronización de precisión a 60.098 FPS
            if (now > nextFrameTime + std::chrono::milliseconds(100)) {
                nextFrameTime = now;
            }
            nextFrameTime += std::chrono::duration_cast<Clock::duration>(
                std::chrono::duration<double, std::milli>(1000.0 / TARGET_FPS)
            );
            if (nextFrameTime > Clock::now()) {
                std::this_thread::sleep_until(nextFrameTime);
            }
        }
    });

    // ==================================================
    // HILO PRINCIPAL: EVENTOS SDL3 + OPENGL + IMGUI
    // ==================================================
    NES::Graphics::FrameBuffer localFrameBuffer;
    NES::GUI::DebugSnapshot localSnapshot;
    uint8_t currentButtons = 0;

    while (appRunning.load(std::memory_order_relaxed))
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_QUIT) {
                appRunning = false;
            }

            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                event.window.windowID == SDL_GetWindowID(window)) {
                appRunning = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP)
            {
                bool isPressed = (event.type == SDL_EVENT_KEY_DOWN);
                SDL_Keycode key = event.key.key;

                // Si la ventana de Configuraciones está esperando una tecla para reasignar:
                if (isPressed && gui.handleKeyCapture(key)) {
                    continue;
                }

                auto updateBit = [&](uint8_t bitIndex, bool pressed) {
                    if (pressed) currentButtons |= (1 << bitIndex);
                    else         currentButtons &= ~(1 << bitIndex);
                };

                if (key == gui.keyMap.a)      updateBit(0, isPressed);
                if (key == gui.keyMap.b)      updateBit(1, isPressed);
                if (key == gui.keyMap.select) updateBit(2, isPressed);
                if (key == gui.keyMap.start)  updateBit(3, isPressed);
                if (key == gui.keyMap.up)     updateBit(4, isPressed);
                if (key == gui.keyMap.down)   updateBit(5, isPressed);
                if (key == gui.keyMap.left)   updateBit(6, isPressed);
                if (key == gui.keyMap.right)  updateBit(7, isPressed);

                atomicControllerState.store(currentButtons, std::memory_order_relaxed);

                if (isPressed && !event.key.repeat) {
                    if (key == SDLK_P)  gui.cmdTogglePause = true;
                    if (key == SDLK_F)  gui.cmdStepFrame   = true;
                    if (key == SDLK_F5) {
                        if (event.key.mod & SDL_KMOD_SHIFT)
                            gui.openSaveStateAsDialog(window);
                        else
                            gui.triggerSaveState(window);
                    }
                    if (key == SDLK_F9) {
                        if (event.key.mod & SDL_KMOD_SHIFT)
                            gui.openLoadStateDialog(window);
                        else
                            gui.triggerLoadLastSaveState();
                    }

                    if ((event.key.mod & SDL_KMOD_CTRL) && key == SDLK_O) {
                        gui.openNativeFileDialog(window);
                    }

                    if ((event.key.mod & SDL_KMOD_CTRL) && key == SDLK_R) {
                        NES::GUI::EmulationState st;
                        {
                            std::lock_guard<std::mutex> lock(gui.stateMutex);
                            st = gui.currentState;
                        }
                        // Solo permite reiniciar si el emulador está corriendo o en pausa
                        if (st == NES::GUI::EmulationState::Running || st == NES::GUI::EmulationState::Paused) {
                            gui.cmdReset = true;
                        }
                    }
                }
            }
        }

        if (!appRunning.load()) break;

        // 1. Copiar último FrameBuffer y Snapshot de forma rápida
        {
            std::lock_guard<std::mutex> lock(frameMutex);
            localFrameBuffer = sharedFrameBuffer;
            localSnapshot = sharedDebugSnapshot;
            sharedDebugSnapshot.patternTablesUpdated = false;
        }

        // 2. Construir frame de Dear ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        gui.render(window, localSnapshot);

        // 3. Limpiar pantalla completa y acotar el área de OpenGL entre las 2 barras
        int winW = 1024, winH = 960;
        SDL_GetWindowSizeInPixels(window, &winW, &winH);

        glDisable(GL_SCISSOR_TEST);
        glViewport(0, 0, winW, winH);
        glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Calcular el rectángulo interior libre (sin pisar la barra superior ni la inferior)
        auto area = gui.getAvailableOpenGLViewport(winW, winH);

        glEnable(GL_SCISSOR_TEST);
        glScissor(area.x, area.y, area.width, area.height);
        glViewport(area.x, area.y, area.width, area.height);

        // Renderizar la pantalla de la NES únicamente si hay ejecución activa o en pausa
        {
            NES::GUI::EmulationState st;
            {
                std::lock_guard<std::mutex> lock(gui.stateMutex);
                st = gui.currentState;
            }
            if (st == NES::GUI::EmulationState::Running || st == NES::GUI::EmulationState::Paused) {
                renderer.renderFrame(localFrameBuffer, area.x, area.y, area.width, area.height);
            }
        }

        glDisable(GL_SCISSOR_TEST);

        // 4. Restaurar viewport completo y pintar las barras de ImGui encima del marco
        glViewport(0, 0, winW, winH);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        #ifdef ImGuiConfigFlags_ViewportsEnable
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            SDL_Window* backupWin = SDL_GL_GetCurrentWindow();
            SDL_GLContext backupCtx = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backupWin, backupCtx);
        }
        #endif

        // Restaurar viewport completo y pintar las barras de ImGui encima del marco
        glViewport(0, 0, winW, winH);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            SDL_Window* backupWin = SDL_GL_GetCurrentWindow();
            SDL_GLContext backupCtx = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backupWin, backupCtx);
        }

        SDL_GL_SwapWindow(window);
    }

    // Esperar a que termine limpiamente el hilo de emulación antes de destruir recursos
    appRunning = false;
    if (emulationThread.joinable()) {
        emulationThread.join();
    }

    gui.destroyOpenGLResources();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    closeAudio();
    renderer.destroy();
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}