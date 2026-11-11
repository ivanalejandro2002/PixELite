#pragma once
#include <imgui.h>
#include <SDL3/SDL.h>
#include <glad/gl.h>

#include <NES/NES.h>
#include <Graphics/FrameBuffer.h>

#include <string>
#include <vector>
#include <array>
#include <filesystem>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <cstdio>

namespace NES::GUI {

    enum class EmulationState {
        NoROM,      // "Sin ROM"
        Running,    // "En ejecución"
        Paused,     // "En pausa"
        Stopped     // "Detenido"
    };

    // Snapshot thread-safe que el hilo de emulación actualiza 1 vez por frame
    // para que el hilo de ImGui/OpenGL lo lea sin bloquear la CPU/PPU
    struct DebugSnapshot {
        // Registros CPU
        uint8_t a = 0, x = 0, y = 0, s = 0xFD, status = 0x24;
        uint16_t pc = 0;

        // Registros PPU ($2000-$2007 y Loopy)
        uint8_t ppuCtrl = 0, ppuMask = 0, ppuStatus = 0, oamAddr = 0;
        uint16_t v = 0, t = 0;
        uint8_t fineX = 0;
        bool w = false;

        // Información del ROM
        uint16_t mapperId = 0;
        size_t prgSizeKB = 0;
        size_t chrSizeKB = 0;

        // Copia de los 2 KB de RAM interna ($0000 - $07FF)
        std::array<uint8_t, 2048> cpuRam{};

        // Buffers RGBA (128x128 píxeles = 65536 bytes cada una) para Pattern Tables 0 y 1
        std::array<uint8_t, 128 * 128 * 4> patternTable0{};
        std::array<uint8_t, 128 * 128 * 4> patternTable1{};
        bool patternTablesUpdated = false;
    };

    struct ControllerKeyMap {
        SDL_Keycode up     = SDLK_UP;
        SDL_Keycode down   = SDLK_DOWN;
        SDL_Keycode left   = SDLK_LEFT;
        SDL_Keycode right  = SDLK_RIGHT;
        SDL_Keycode a      = SDLK_Z;
        SDL_Keycode b      = SDLK_X;
        SDL_Keycode select = SDLK_RSHIFT;
        SDL_Keycode start  = SDLK_RETURN;
    };

    struct ViewportRect {
        int x = 0;
        int y = 0;
        int width = 256;
        int height = 240;
    };

    class EmulatorGUI {
    public:
        // Checkboxes de la pestaña Debug
        bool showPatternTables = false;
        bool showCPURegisters  = false;
        bool showRAM           = false;
        bool showPPURegisters  = false;

        // Ventanas secundarias
        bool showRomInfoWindow     = false;
        bool showControllerConfig  = false;
        bool showAboutWindow       = false;

        // Alturas de las barras para recortar el glViewport
        float topBarHeight    = 22.0f;
        float bottomBarHeight = 26.0f;

        // Mapeo de teclas y estado de reasignación
        ControllerKeyMap keyMap;
        int waitingForKeyIndex = -1;

        // Texturas OpenGL para las 2 Pattern Tables
        GLuint patternTex[2] = {0, 0};

        // Sincronización con el hilo de emulación
        mutable std::mutex stateMutex;
        EmulationState currentState = EmulationState::NoROM;
        std::string currentRomPath  = "";
        std::string currentRomName  = "No cargada";
        std::vector<std::string> recentRoms; // Máximo 8

        // Comandos hacia el hilo de emulación
        std::atomic<bool> cmdLoadRom{false};
        std::string pendingRomPath = "";
        std::atomic<bool> cmdStart{false};
        std::atomic<bool> cmdStop{false};
        std::atomic<bool> cmdReset{false};
        std::atomic<bool> cmdTogglePause{false};
        std::atomic<bool> cmdStepFrame{false};
        std::atomic<float> emulationFps{0.0f};

        void initOpenGLResources() {
            glGenTextures(2, patternTex);
            for (int i = 0; i < 2; ++i) {
                glBindTexture(GL_TEXTURE_2D, patternTex[i]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        void destroyOpenGLResources() {
            if (patternTex[0] != 0 || patternTex[1] != 0) {
                glDeleteTextures(2, patternTex);
                patternTex[0] = patternTex[1] = 0;
            }
        }

        // Llamado por el diálogo nativo de SDL3 o al elegir una ROM
        void requestLoadRom(const std::string& path) {
            std::lock_guard<std::mutex> lock(stateMutex);
            pendingRomPath = path;
            cmdLoadRom = true;
        }

        // Llamado desde el hilo de emulación cuando la ROM carga con éxito
        void confirmRomLoaded(const std::string& filepath) {
            std::lock_guard<std::mutex> lock(stateMutex);
            currentRomPath = filepath;
            std::filesystem::path p(filepath);
            currentRomName = p.stem().string(); // Nombre limpio sin ".nes"
            currentState   = EmulationState::Running;

            auto it = std::find(recentRoms.begin(), recentRoms.end(), filepath);
            if (it != recentRoms.end()) recentRoms.erase(it);
            recentRoms.insert(recentRoms.begin(), filepath);
            if (recentRoms.size() > 8) recentRoms.resize(8);
        }

        ViewportRect getAvailableOpenGLViewport(int windowWidth, int windowHeight) const {
            int availHeight = windowHeight - static_cast<int>(topBarHeight) - static_cast<int>(bottomBarHeight);
            if (availHeight < 0) availHeight = 0;
            return {
                0,
                static_cast<int>(bottomBarHeight),
                windowWidth,
                availHeight
            };
        }

        // Captura una tecla si el usuario está reasignando controles en Configuraciones
        bool handleKeyCapture(SDL_Keycode key) {
            if (waitingForKeyIndex < 0) return false;
            switch (waitingForKeyIndex) {
                case 0: keyMap.up     = key; break;
                case 1: keyMap.down   = key; break;
                case 2: keyMap.left   = key; break;
                case 3: keyMap.right  = key; break;
                case 4: keyMap.a      = key; break;
                case 5: keyMap.b      = key; break;
                case 6: keyMap.select = key; break;
                case 7: keyMap.start  = key; break;
            }
            waitingForKeyIndex = -1;
            return true;
        }

        // Callback estático para SDL3_ShowOpenFileDialog
        static void SDLCALL onFileDialogCallback(void* userdata, const char* const* filelist, int filter) {
            (void)filter;
            if (!filelist || !filelist[0]) return; // Usuario canceló
            auto* gui = static_cast<EmulatorGUI*>(userdata);
            gui->requestLoadRom(filelist[0]);
        }

        void openNativeFileDialog(SDL_Window* window) {
            static const SDL_DialogFileFilter filters[] = {
                { "Archivos NES ROM (.nes)", "nes" },
                { "Todos los archivos", "*" }
            };
            SDL_ShowOpenFileDialog(onFileDialogCallback, this, window, filters, 2, nullptr, false);
        }

        void render(SDL_Window* window, const DebugSnapshot& snap) {
            updateResponsiveScale(window);
            renderTopMenuBar(window);
            renderBottomStatusBar();
            renderSecondaryWindows(snap);
            renderDebugWindows(snap);
        }

    private:
        ImGuiStyle baseStyle;
        bool baseStyleInitialized = false;
        float currentUiScale = 1.0f;

        void updateResponsiveScale(SDL_Window* window) {
            if (!baseStyleInitialized) {
                baseStyle = ImGui::GetStyle();
                baseStyleInitialized = true;
            }

            float displayScale = SDL_GetWindowDisplayScale(window);
            if (displayScale <= 0.0f) displayScale = 1.0f;

            // Mantenemos la escala global estable en 1.0x (o el DPI del monitor)
            // para que las barras superior/inferior y títulos de ventana tengan tamaño normal:
            float targetScale = std::clamp(displayScale, 1.0f, 1.5f);

            if (std::abs(targetScale - currentUiScale) > 0.01f) {
                currentUiScale = targetScale;
                ImGuiStyle& style = ImGui::GetStyle();
                style = baseStyle;
                style.ScaleAllSizes(currentUiScale);
                ImGui::GetIO().FontGlobalScale = currentUiScale;
            }
        }

        // Ajusta el tamaño de letra para que 'sampleLongestLine' y 'lineCount'
        // encajen exactamente dentro del área interior actual de la subventana:
        float fitFontToWindow(const char* sampleLongestLine, int lineCount) {
            ImGui::SetWindowFontScale(1.0f); // Resetear primero para medir sin distorsión

            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImVec2 baseTextSize = ImGui::CalcTextSize(sampleLongestLine);
            float baseHeight = ImGui::GetTextLineHeightWithSpacing() * static_cast<float>(lineCount);

            if (baseTextSize.x <= 0.0f || baseHeight <= 0.0f) return 1.0f;

            // Dejamos un 8% de margen de respiración para que nunca toque los bordes
            float scaleX = (avail.x * 0.92f) / baseTextSize.x;
            float scaleY = (avail.y * 0.92f) / baseHeight;

            float scale = std::clamp(std::min(scaleX, scaleY), 0.45f, 1.6f);
            ImGui::SetWindowFontScale(scale);
            return scale;
        }

        const char* getStateText(EmulationState st) const {
            switch (st) {
                case EmulationState::Running: return "En ejecución";
                case EmulationState::Paused:  return "En pausa";
                case EmulationState::NoROM:   return "Sin ROM";
                case EmulationState::Stopped: return "Detenido";
                default:                      return "Detenido";
            }
        }

        void renderTopMenuBar(SDL_Window* window) {
            if (ImGui::BeginMainMenuBar()) {
                topBarHeight = ImGui::GetWindowSize().y;

                EmulationState st;
                bool hasRom;
                std::vector<std::string> recentsCopy;
                {
                    std::lock_guard<std::mutex> lock(stateMutex);
                    st = currentState;
                    hasRom = !currentRomPath.empty();
                    recentsCopy = recentRoms;
                }

                // 1. ARCHIVO
                if (ImGui::BeginMenu("Archivo")) {
                    if (ImGui::MenuItem("Cargar ROM", "Ctrl+O")) {
                        openNativeFileDialog(window);
                    }
                    if (ImGui::MenuItem("Información del ROM", nullptr, false, hasRom)) {
                        showRomInfoWindow = true;
                    }
                    ImGui::Separator();

                    bool canStart = (st == EmulationState::Stopped || st == EmulationState::NoROM) && hasRom;
                    if (ImGui::MenuItem("Iniciar Ejecución", nullptr, false, canStart)) {
                        cmdStart = true;
                    }

                    bool canStop = (st == EmulationState::Running || st == EmulationState::Paused);
                    if (ImGui::MenuItem("Terminar Ejecución", nullptr, false, canStop)) {
                        cmdStop = true;
                    }
                    ImGui::Separator();

                    // Aún no implementados funcionalmente según especificación, listos en UI:
                    if (ImGui::MenuItem("Escoger dirección de ROM")) {}

                    if (ImGui::BeginMenu("ROMs recientes")) {
                        if (recentsCopy.empty()) {
                            ImGui::MenuItem("(Sin ROMs recientes)", nullptr, false, false);
                        } else {
                            for (size_t i = 0; i < recentsCopy.size(); ++i) {
                                std::string itemLabel = std::to_string(i + 1) + ". " + recentsCopy[i];
                                if (ImGui::MenuItem(itemLabel.c_str())) {
                                    requestLoadRom(recentsCopy[i]);
                                }
                            }
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }

                // 2. SISTEMA
                if (ImGui::BeginMenu("Sistema")) {
                    bool isRunningOrPaused = (st == EmulationState::Running || st == EmulationState::Paused);

                    if (ImGui::MenuItem("Reiniciar", "Ctrl+R", false, isRunningOrPaused)) {
                        cmdReset = true;
                    }
                    if (ImGui::MenuItem("Pausar / Continuar", "P", false, isRunningOrPaused)) {
                        cmdTogglePause = true;
                    }
                    if (ImGui::MenuItem("Avanzar un frame", "F", false, isRunningOrPaused)) {
                        cmdStepFrame = true;
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Guardar Estado", "F5", false, isRunningOrPaused)) {}
                    if (ImGui::MenuItem("Guardar Estado Como...", "Shift+F5", false, isRunningOrPaused)) {}
                    ImGui::EndMenu();
                }

                // 3. CONFIGURACIONES
                if (ImGui::BeginMenu("Configuraciones")) {
                    if (ImGui::MenuItem("Configuración de controles")) {
                        showControllerConfig = true;
                    }
                    ImGui::EndMenu();
                }

                // 4. DEBUG
                if (ImGui::BeginMenu("Debug")) {
                    ImGui::MenuItem("Pattern Tables", nullptr, &showPatternTables);
                    ImGui::MenuItem("CPU Registers",  nullptr, &showCPURegisters);
                    ImGui::MenuItem("RAM",            nullptr, &showRAM);
                    ImGui::MenuItem("PPURegisters",   nullptr, &showPPURegisters);
                    ImGui::EndMenu();
                }

                // 5. AYUDA
                if (ImGui::BeginMenu("Ayuda")) {
                    if (ImGui::MenuItem("Acerca de PixELite")) {
                        showAboutWindow = true;
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }
        }

        void renderBottomStatusBar() {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            bottomBarHeight = ImGui::GetFrameHeight() + 4.0f;

            // Anclar obligatoriamente dentro del Viewport de la ventana principal
            // (evita que Multi-Viewport cree una ventana del SO separada para la barra):
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - bottomBarHeight));
            ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, bottomBarHeight));

            ImGuiWindowFlags flags =
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoDocking |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 3.0f));

            if (ImGui::Begin("##BottomStatusBar", nullptr, flags)) {
                ImGui::SetWindowFontScale(1.0f);

                EmulationState st;
                std::string romLabel;
                {
                    std::lock_guard<std::mutex> lock(stateMutex);
                    st = currentState;
                    romLabel = (st == EmulationState::NoROM || st == EmulationState::Stopped)
                        ? "ROM no cargada"
                        : currentRomName;
                }

                // Esquina inferior izquierda
                ImGui::Text("Estado: %s", getStateText(st));

                // Esquina inferior derecha -> FPS: <FPS> : ROM : <nombre>
                float fpsVal = (st == EmulationState::Running) ? emulationFps.load() : 0.0f;
                char rightBuf[256];
                std::snprintf(rightBuf, sizeof(rightBuf), "FPS: %.1f : ROM : %s", fpsVal, romLabel.c_str());

                float textWidth = ImGui::CalcTextSize(rightBuf).x;
                ImGui::SameLine(ImGui::GetWindowWidth() - textWidth - 14.0f);
                ImGui::TextUnformatted(rightBuf);
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
        }

        // Calcula y aplica una escala de texto proporcional al tamaño actual de la subventana
        void applyDynamicWindowFontScale(float baseWidth, float baseHeight) {
            ImVec2 size = ImGui::GetWindowSize();
            float scaleX = size.x / baseWidth;
            float scaleY = size.y / baseHeight;

            // Usamos el menor de los dos ejes para que el texto nunca se desborde ni horizontal ni verticalmente
            float factor = std::min(scaleX, scaleY);

            // Acotamos entre 0.55x (ventana muy chica) y 2.5x (ventana muy grande)
            factor = std::clamp(factor, 0.55f, 2.5f);

            ImGui::SetWindowFontScale(factor);
        }

        void renderSecondaryWindows(const DebugSnapshot& snap) {
            if (showControllerConfig) {
                ImGui::SetNextWindowSize(ImVec2(380.0f, 320.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f, 170.0f), ImVec2(FLT_MAX, FLT_MAX));
                if (ImGui::Begin("Configuración de Controles", &showControllerConfig)) {
                    fitFontToWindow("Asignación de teclas del Control Original:   ", 10);
                    const float localEm = ImGui::GetFontSize();

                    ImGui::Text("Asignación de teclas del Control Original:");
                    ImGui::Separator();

                    auto row = [&](const char* name, SDL_Keycode code, int idx) {
                        ImGui::Text("%-10s", name);
                        ImGui::SameLine(localEm * 7.2f);
                        std::string label = (waitingForKeyIndex == idx)
                            ? "[ Presiona una tecla... ]"
                            : SDL_GetKeyName(code);
                        if (ImGui::Button((label + "##btn" + std::to_string(idx)).c_str(), ImVec2(localEm * 11.5f, 0))) {
                            waitingForKeyIndex = idx;
                        }
                    };

                    row("Arriba",    keyMap.up,     0);
                    row("Abajo",     keyMap.down,   1);
                    row("Izquierda", keyMap.left,   2);
                    row("Derecha",   keyMap.right,  3);
                    row("Botón A",   keyMap.a,      4);
                    row("Botón B",   keyMap.b,      5);
                    row("Select",    keyMap.select, 6);
                    row("Start",     keyMap.start,  7);
                    ImGui::SetWindowFontScale(1.0f);
                }
                ImGui::End();
            }

            if (showRomInfoWindow) {
                ImGui::SetNextWindowSize(ImVec2(350.0f, 190.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSizeConstraints(ImVec2(180.0f, 110.0f), ImVec2(FLT_MAX, FLT_MAX));
                if (ImGui::Begin("Información del ROM", &showRomInfoWindow)) {
                    fitFontToWindow("Nombre: Super Mario Bros. 3 (USA)", 7);

                    std::lock_guard<std::mutex> lock(stateMutex);
                    ImGui::Text("Nombre: %s", currentRomName.c_str());
                    ImGui::TextWrapped("Ruta: %s", currentRomPath.c_str());
                    ImGui::Separator();
                    ImGui::Text("Mapper ID: %u", snap.mapperId);
                    ImGui::Text("PRG-ROM:   %zu KB", snap.prgSizeKB);
                    ImGui::Text("CHR:       %zu KB", snap.chrSizeKB);
                    ImGui::SetWindowFontScale(1.0f);
                }
                ImGui::End();
            }

            if (showAboutWindow) {
                ImGui::SetNextWindowSize(ImVec2(340.0f, 130.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSizeConstraints(ImVec2(180.0f, 80.0f), ImVec2(FLT_MAX, FLT_MAX));
                if (ImGui::Begin("Ayuda - Acerca de", &showAboutWindow)) {
                    fitFontToWindow("PixELite - Emulador Ciclo-Exacto de NES", 4);

                    ImGui::Text("PixELite - Emulador Ciclo-Exacto de NES");
                    ImGui::Separator();
                    ImGui::Text("Arquitectura Ricoh 2A03 / PPU 2C02");
                    ImGui::SetWindowFontScale(1.0f);
                }
                ImGui::End();
            }
        }

        void renderDebugWindows(const DebugSnapshot& snap) {
            // 1. CPU REGISTERS
            if (showCPURegisters) {
                ImGui::SetNextWindowSize(ImVec2(280.0f, 210.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSizeConstraints(ImVec2(140.0f, 110.0f), ImVec2(FLT_MAX, FLT_MAX));
                if (ImGui::Begin("CPU Registers", &showCPURegisters)) {
                    fitFontToWindow("N:0 V:0 -:1 B:0 D:0 I:1 Z:0 C:1", 9);

                    ImGui::Text("A:  $%02X (%3d)", snap.a, snap.a);
                    ImGui::Text("X:  $%02X (%3d)", snap.x, snap.x);
                    ImGui::Text("Y:  $%02X (%3d)", snap.y, snap.y);
                    ImGui::Separator();
                    ImGui::Text("PC: $%04X", snap.pc);
                    ImGui::Text("S:  $01%02X", snap.s);
                    ImGui::Separator();
                    ImGui::Text("Status (NV-BDIZC): $%02X", snap.status);
                    ImGui::Text("N:%d V:%d -:%d B:%d D:%d I:%d Z:%d C:%d",
                        (snap.status >> 7) & 1, (snap.status >> 6) & 1,
                        (snap.status >> 5) & 1, (snap.status >> 4) & 1,
                        (snap.status >> 3) & 1, (snap.status >> 2) & 1,
                        (snap.status >> 1) & 1, snap.status & 1);

                    ImGui::SetWindowFontScale(1.0f); // Mantiene la barra de título en su tamaño normal
                }
                ImGui::End();
            }

            // 2. PPU REGISTERS
            if (showPPURegisters) {
                ImGui::SetNextWindowSize(ImVec2(300.0f, 240.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSizeConstraints(ImVec2(150.0f, 120.0f), ImVec2(FLT_MAX, FLT_MAX));
                if (ImGui::Begin("PPURegisters", &showPPURegisters)) {
                    fitFontToWindow("Registros de Scrolling:   ", 11);

                    ImGui::Text("Registros $2000 - $2007:");
                    ImGui::Separator();
                    ImGui::Text("$2000 (PPUCTRL):   $%02X", snap.ppuCtrl);
                    ImGui::Text("$2001 (PPUMASK):   $%02X", snap.ppuMask);
                    ImGui::Text("$2002 (PPUSTATUS): $%02X", snap.ppuStatus);
                    ImGui::Text("$2003 (OAMADDR):   $%02X", snap.oamAddr);
                    ImGui::Separator();
                    ImGui::Text("Registros de Scrolling:");
                    ImGui::Text("v (VRAM Addr):  $%04X", snap.v);
                    ImGui::Text("t (Temp Addr):  $%04X", snap.t);
                    ImGui::Text("Fine X:         %u", snap.fineX);
                    ImGui::Text("Write Toggle w: %d", snap.w ? 1 : 0);

                    ImGui::SetWindowFontScale(1.0f);
                }
                ImGui::End();
            }

            // 3. RAM ($0000 - $07FF)
            if (showRAM) {
                ImGui::SetNextWindowSize(ImVec2(540.0f, 360.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSizeConstraints(ImVec2(220.0f, 120.0f), ImVec2(FLT_MAX, FLT_MAX));
                if (ImGui::Begin("RAM ($0000 - $07FF)", &showRAM)) {
                    ImGui::SetWindowFontScale(1.0f);
                    // Medimos exactamente una fila hexadecimal de 16 bytes + barra de scroll:
                    const char* sampleHexRow = "$0000: 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00   ";
                    float baseRowWidth = ImGui::CalcTextSize(sampleHexRow).x;
                    float availW = ImGui::GetContentRegionAvail().x;
                    float ramScale = std::clamp((availW * 0.95f) / baseRowWidth, 0.40f, 1.5f);

                    ImGui::BeginChild("RamHexRegion", ImVec2(0, 0), true);
                    ImGui::SetWindowFontScale(ramScale);
                    ImGuiListClipper clipper;
                    clipper.Begin(2048 / 16);
                    while (clipper.Step()) {
                        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                            uint16_t base = static_cast<uint16_t>(row * 16);
                            ImGui::Text("$%04X: %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X",
                                base,
                                snap.cpuRam[base + 0],  snap.cpuRam[base + 1],  snap.cpuRam[base + 2],  snap.cpuRam[base + 3],
                                snap.cpuRam[base + 4],  snap.cpuRam[base + 5],  snap.cpuRam[base + 6],  snap.cpuRam[base + 7],
                                snap.cpuRam[base + 8],  snap.cpuRam[base + 9],  snap.cpuRam[base + 10], snap.cpuRam[base + 11],
                                snap.cpuRam[base + 12], snap.cpuRam[base + 13], snap.cpuRam[base + 14], snap.cpuRam[base + 15]);
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::End();
            }

            // 4. PATTERN TABLES
            if (showPatternTables) {
                if (snap.patternTablesUpdated) {
                    glBindTexture(GL_TEXTURE_2D, patternTex[0]);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 128, GL_RGBA, GL_UNSIGNED_BYTE, snap.patternTable0.data());
                    glBindTexture(GL_TEXTURE_2D, patternTex[1]);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 128, GL_RGBA, GL_UNSIGNED_BYTE, snap.patternTable1.data());
                    glBindTexture(GL_TEXTURE_2D, 0);
                }

                ImGui::SetNextWindowSize(ImVec2(540.0f, 310.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSizeConstraints(ImVec2(220.0f, 140.0f), ImVec2(FLT_MAX, FLT_MAX));

                if (ImGui::Begin("Pattern Tables", &showPatternTables)) {
                    ImGui::SetWindowFontScale(1.0f);

                    ImVec2 avail = ImGui::GetContentRegionAvail();
                    float spacing = ImGui::GetStyle().ItemSpacing.x;

                    // Primero calculamos el tamaño exacto de cada imagen cuadrada (tablePx)
                    float maxByWidth  = (avail.x - spacing) * 0.5f;
                    float maxByHeight = avail.y - 26.0f;
                    float tablePx = std::max(48.0f, std::min(maxByWidth, maxByHeight));

                    // Escalamos la etiqueta para que NUNCA sea más ancha que tablePx
                    float labelBaseW = ImGui::CalcTextSize("Pattern Table 0 ($0000)").x;
                    float labelScale = std::clamp((tablePx * 0.95f) / labelBaseW, 0.45f, 1.15f);
                    ImGui::SetWindowFontScale(labelScale);

                    // Recalcular tablePx con la altura real que quedó en la etiqueta
                    maxByHeight = avail.y - ImGui::GetTextLineHeightWithSpacing() - 4.0f;
                    tablePx = std::max(48.0f, std::min(maxByWidth, maxByHeight));

                    ImGui::BeginGroup();
                    ImGui::Text("Pattern Table 0 ($0000)");
                    ImGui::Image((ImTextureID)(intptr_t)patternTex[0], ImVec2(tablePx, tablePx));
                    ImGui::EndGroup();

                    ImGui::SameLine(tablePx + spacing + ImGui::GetWindowContentRegionMin().x);

                    ImGui::BeginGroup();
                    ImGui::Text("Pattern Table 1 ($1000)");
                    ImGui::Image((ImTextureID)(intptr_t)patternTex[1], ImVec2(tablePx, tablePx));
                    ImGui::EndGroup();

                    ImGui::SetWindowFontScale(1.0f);
                }
                ImGui::End();
            }
        }
    };

}