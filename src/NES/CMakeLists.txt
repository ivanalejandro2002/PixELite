add_library(NES

    src/CPU/MOS6502.cpp
    src/CPU/StatusRegister.cpp
    src/CPU/ALU.cpp
)

target_include_directories(NES PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
