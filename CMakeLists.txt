cmake_minimum_required(VERSION 3.20)

# Definir política y runtime estático ANTES de project(TT) para que SDL3, ImGui y TT usen el mismo Heap:
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(TT)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_compile_definitions(PROJECT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Soporte UTF-8 en MSVC (Windows) para acentos y caracteres en español
if(MSVC)
    add_compile_options(/utf-8 /EHsc)
    add_compile_definitions(NOMINMAX _CRT_SECURE_NO_WARNINGS uint=unsigned)
    add_link_options(/STACK:8388608)
endif()

# =========================================
# SDL3 Static Configuration & X11/Wayland
# =========================================

set(SDL_X11_XSCRNSAVER OFF CACHE BOOL "" FORCE)
set(SDL_X11_XTEST OFF CACHE BOOL "" FORCE)
set(SDL_X11_XFIXES OFF CACHE BOOL "" FORCE)
set(SDL_X11_XCURSOR OFF CACHE BOOL "" FORCE)
set(SDL_X11_XSHAPE OFF CACHE BOOL "" FORCE)

# Forzar SDL3 estático ANTES de add_subdirectory para no depender de libSDL3.so.0 ni SDL3.dll
set(SDL_SHARED OFF CACHE BOOL "Disable SDL3 shared library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Enable SDL3 static library" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Disable SDL3 test library" FORCE)

add_subdirectory(external/SDL)

# =========================================
# OpenGL
# =========================================

find_package(OpenGL REQUIRED)

# =========================================
# GLAD2
# =========================================

add_library(glad
    external/glad/src/gl.c
)

target_include_directories(glad PUBLIC
    external/glad/include
)

# =========================================
# Dear ImGui
# =========================================

add_library(imgui
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp

    external/imgui/backends/imgui_impl_sdl3.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
    external/SDL/include
)

target_compile_definitions(imgui PUBLIC
    IMGUI_IMPL_OPENGL_LOADER_CUSTOM
)

if(MSVC)
    target_compile_options(imgui PUBLIC "/FIglad/gl.h")
else()
    target_compile_options(imgui PUBLIC "-include" "glad/gl.h")
endif()

# =========================================
# Link ImGui Dependencies
# =========================================

target_link_libraries(imgui PUBLIC
    SDL3::SDL3-static
    glad
    OpenGL::GL
)

# =========================================
# Internal Modules
# =========================================

add_subdirectory(src/NES)

# =========================================
# Main Executable
# =========================================

add_executable(TT
    src/main.cpp
    src/NES/src/Graphics/Renderer.cpp
)

# =========================================
# Link Everything
# =========================================

target_link_libraries(TT PRIVATE
    SDL3::SDL3-static
    imgui
    glad
    OpenGL::GL
    NES
)

if(UNIX AND NOT APPLE)
    target_link_options(TT PRIVATE -static-libgcc -static-libstdc++)
    target_link_libraries(TT PRIVATE pthread dl)
endif()