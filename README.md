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