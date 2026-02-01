include(FetchContent)

# FTXUI - Terminal UI library
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)

# Box2D v3 - Physics engine
set(BOX2D_SAMPLES OFF CACHE BOOL "" FORCE)
set(BOX2D_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BOX2D_DOCS OFF CACHE BOOL "" FORCE)
set(BOX2D_PROFILE OFF CACHE BOOL "" FORCE)
set(BOX2D_VALIDATE OFF CACHE BOOL "" FORCE)
set(BOX2D_UNIT_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.1.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

# stb - Single-file public domain libraries (stb_image for PNG loading)
FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(ftxui box2d stb)

# Create interface target for stb_image (header-only)
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_SOURCE_DIR})

# Suppress -Werror=maybe-uninitialized in Box2D (GCC false positive in optimized builds)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(box2d PRIVATE -Wno-error=maybe-uninitialized)
endif()

# libgamepad - optional gamepad support
option(ENABLE_GAMEPAD "Enable gamepad support via libgamepad" OFF)
if(ENABLE_GAMEPAD)
    FetchContent_Declare(libgamepad
        GIT_REPOSITORY https://github.com/univrsal/libgamepad.git
        GIT_TAG master
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(libgamepad)
endif()
