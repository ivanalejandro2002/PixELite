add_library(NES

    src/CPU/MOS6502.cpp
    src/CPU/StatusRegister.cpp
    src/CPU/ALU.cpp

    src/CPU/AddressingModes.cpp
    src/CPU/Instruction.cpp
    src/CPU/InstructionFactory.cpp
    src/CPU/InstructionTable.cpp
    src/CPU/InstructionBuilder.cpp

    src/PPU/CIRAM.cpp
    src/PPU/PaletteRAM.cpp
    src/PPU/PPU2C02.cpp
    src/PPU/PPURegisters.cpp

    src/Memory/RAM.cpp
    src/Memory/PRGROM.cpp

    src/Cartridge/CHRROM.cpp

    src/Bus.cpp

    src/NES.cpp

    src/Graphics/FrameBuffer.cpp
)

target_include_directories(NES PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
