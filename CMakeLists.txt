cmake_minimum_required(VERSION 3.20)

project(TT)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================================
# SDL3 Optional Linux/X11 Dependencies
# =========================================

set(SDL_X11_XSCRNSAVER OFF CACHE BOOL "" FORCE)
set(SDL_X11_XTEST OFF CACHE BOOL "" FORCE)
set(SDL_X11_XFIXES OFF CACHE BOOL "" FORCE)
set(SDL_X11_XCURSOR OFF CACHE BOOL "" FORCE)
set(SDL_X11_XSHAPE OFF CACHE BOOL "" FORCE)

# =========================================
# SDL3
# =========================================

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

# =========================================
# Link ImGui Dependencies
# =========================================

target_link_libraries(imgui PUBLIC
    SDL3::SDL3
    glad
    OpenGL::GL
)

# =========================================
# Internal Modules
# =========================================

# add_subdirectory(src/Módulo)
add_subdirectory(src/NES)


# =========================================
# Main Executable
# =========================================

add_executable(TT
    src/main.cpp
)

# =========================================
# Link Everything
# =========================================

target_link_libraries(TT PRIVATE
    SDL3::SDL3
    imgui
    glad
    OpenGL::GL
    NES
)

# =========================================
# Linux pthread fix
# =========================================

if(UNIX AND NOT APPLE)
    target_link_libraries(TT PRIVATE pthread dl)
endif()