# PixELite 🎮

**PixELite** es un emulador de NES (Nintendo Entertainment System) de alta fidelidad y **precisión a nivel de ciclos (Cycle-Accurate)**, desarrollado en C++. Su arquitectura modela con rigor científico el silicio original de la consola, incluyendo el núcleo de CPU Ricoh 2A03 (MOS 6502), la Unidad de Procesamiento de Video Ricoh 2C02 (PPU), la Unidad de Procesamiento de Audio (APU) con mezclador no lineal y un subsistema de gráficos y depuración avanzado impulsado por **Dear ImGui** y **SDL3**.

---

## 🚀 Características Principales y Operatividad

* **Precisión Ciclo-Exacta:** Sincronización estricta basada en el reloj maestro NTSC (21.477272 MHz), manteniendo una relación exacta de 3 ciclos de PPU por cada 1 ciclo de CPU/APU.
* **Alto Rendimiento en Pruebas de Hardware:** Alcanza un puntaje de **130 casos pasados** en el riguroso banco de pruebas **AccuracyCoin**, validando instrucciones ilegales, temporizaciones de VBlank, comportamiento del bus abierto y gestión avanzada de interrupciones (incluyendo *NMI Hijacking*).
* **Compatibilidad con Mappers:** Soporte robusto para los mappers de cartuchos más importantes de la biblioteca de NES y desarrollo *homebrew*:
  * **Mapper 0 (NROM):** Sin conmutación de bancos (ej. *Super Mario Bros.*, *Donkey Kong*).
  * **Mapper 1 (MMC1 / SxROM):** Registro de desplazamiento en serie de 5 escrituras y nametables dinámicas.
  * **Mapper 2 (UxROM):** Paginación de PRG de 16 KB.
  * **Mapper 4 (MMC3 / TxROM):** Conmutación fina de bancos y contador de IRQ por línea de escaneo basado en flancos de subida de la línea PPU A12 (ej. *Super Mario Bros. 3*).
  * **Mapper 7 (AxROM):** Paginación de PRG de 32 KB y configuración de pantalla única.
  * **Mapper 30 (UNROM 512):** Estándar moderno de NESmaker con soporte de hasta 512 KB de PRG y CHR-RAM paginada.
* **Sistema de Savestates (`.pxsave`):** Serialización binaria determinista que preserva los registros internos, los latches ocultos de tubería y el búfer de video, permitiendo cargar estados instantáneamente incluso con el emulador en pausa.
* **Audio de Alta Fidelidad:** Síntesis precisa de los 5 canales de audio (Pulso 1/2 con unidades *Sweep* asimétricas, Triángulo de 32 pasos, Ruido con LFSR de 15 bits con conmutación de modo largo y corto, y DPCM con DMA autónomo), complementado con filtros pasa-altas/pasa-bajas y control dinámico de velocidad (DRC) para evitar saltos de audio.

---

## 📦 Dependencias de Linux (Ubuntu/Debian)

Con el fin de poder compilar y correr el proyecto, es necesario incluir dependencias de manejadores de ventana, gráficos OpenGL y audio de SDL3. Para esto, ejecuta los siguientes comandos en tu terminal:

```bash
sudo apt install \
build-essential \
cmake \
pkg-config \
libx11-dev \
libxext-dev \
libxrandr-dev \
libxinerama-dev \
libxcursor-dev \
libxi-dev \
libxss-dev \
libwayland-dev \
wayland-protocols \
libegl1-mesa-dev \
libgl1-mesa-dev \
libasound2-dev \
libpulse-dev \
libpipewire-0.3-dev
```

---

## 📥 Clonación e Instalación del Repositorio

Para clonar el repositorio es necesario seguir una serie de pasos importantes, asegurándote de descargar de forma recursiva los submódulos externos del proyecto (`imgui` y `SDL3`):

1. **Clonar el repositorio con submódulos:**
   ```bash
   git clone --recursive https://github.com/ivanalejandro2002/PixELite.git
   cd PixELite
   ```

2. **Configurar el proyecto con CMake en modo Release:**
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   ```

3. **Compilar todos los objetivos en paralelo:**
   ```bash
   cmake --build build -j$(nproc)
   ```

---

## 🖥️ Configuración Especial para Múltiples Ventanas (Multi-Viewport)

PixELite utiliza las capacidades de acoplamiento (*Docking*) y ventanas desacopladas independientes (*Multi-Viewports*) de Dear ImGui para permitirte desacoplar paneles de depuración (Pattern Tables, RAM hexadecimal, registros de CPU/PPU) fuera de la ventana principal.

### Modificación para Entornos Linux (X11 y Wayland / GNOME)
Si ejecutas el emulador en distribuciones Linux modernas utilizando escritorios con Wayland o entornos híbridos XWayland, asegúrate de configurar de manera explícita el backend de video de SDL3 en `src/main.cpp` antes de llamar a `SDL_Init`:

```cpp
#ifdef __linux__
// Fuerza a SDL3 a respaldar X11 y Wayland para permitir ventanas multi-viewport independientes
SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11,wayland");
#endif
```

---

## ⌨️ Controles y Atajos de Teclado

### Mando de NES (Configurable desde la interfaz)
* **Cruceta (Direccionales):** Teclas de Flechas (`Arriba`, `Abajo`, `Izquierda`, `Derecha`)
* **Botón A:** Tecla `Z`
* **Botón B:** Tecla `X`
* **Select:** Tecla `RSHIFT` (Shift Derecho)
* **Start:** Tecla `RETURN` (Enter)

### Atajos Globales de Emulación
* **`Ctrl + O`:** Abrir el explorador para cargar una nueva ROM (`.nes`).
* **`Ctrl + R`:** Reiniciar el sistema emulado (Soft Reset).
* **`P`:** Pausar o reanudar la ejecución.
* **`F`:** Avanzar exactamente un fotograma (*Frame Step*) estando en pausa.
* **`F5`:** Guardar estado rápido en el archivo vinculado actual (`.pxsave`).
* **`Shift + F5`:** Abrir el diálogo "Guardar Estado Como...".
* **`F9`:** Cargar el último savestate vinculado rápidamente (*Cargar último savestate*).
* **`Shift + F9`:** Abrir el explorador para seleccionar un savestate de forma manual.

---

## 📂 Organización de Directorios para ROMs y Savestates

PixELite separa de forma independiente las rutas operativas para evitar conflictos de carpetas:
* **Directorio de ROMs:** Recuerda automáticamente la última carpeta desde donde cargaste un archivo `.nes`.
* **Directorio de Savestates (`/savestates`):** Una ruta exclusiva y separada donde se almacenan por defecto los archivos `.pxsave` asociados al nombre de cada juego.

---

## ⚖️ Licencia

Este proyecto está distribuido bajo los términos de la **Licencia Pública General de GNU v3.0 (GPLv3)**. Consulta el archivo [LICENSE](LICENSE) para más detalles.