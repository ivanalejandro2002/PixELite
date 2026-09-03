## Dependencias de Linux (Ubuntu/Debian)

Con el fin de poder correr el proyecto, es necesario incluir dependencias de manejadores de ventana y archivos. Para esto, es importante descargar las siguientes. Para el caso de X11 o Wayland dependerá del que se desee emplear.

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
libgl1-mesa-dev
```

# Clonación del repositorio

Para clonar el resposiorio es necesario seguir una serie de pasos importantes, esto debido a submódulos externos a este proyecto, los cuales son imgui y SDL3.
* git clone --recursive <liga del repositorio>
* cmake -B build
* cmake --build build
* ./build/TT