add_library(NES

    src/CPU/MOS6502.cpp
    src/CPU/StatusRegister.cpp
    src/CPU/ALU.cpp

    src/Memory/RAM.cpp

    src/Bus.cpp

    src/NES.cpp
)

target_include_directories(NES PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
