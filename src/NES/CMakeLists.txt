file(GLOB_RECURSE NES_SOURCES
    *.cpp
)

add_library(NES
    ${NES_SOURCES}
)

target_include_directories(NES PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)