cmake_minimum_required(VERSION 3.20)
project(MasqueradeBall VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configurable asset directory path
# Default: "assets" (relative path for development builds)
# Override with -DMASQUERADE_ASSETS_DIR=/usr/share/masquerade-ball for install builds
if(NOT DEFINED MASQUERADE_ASSETS_DIR)
    set(MASQUERADE_ASSETS_DIR "assets")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/config.hpp
    @ONLY
)

# Fetch dependencies
include(cmake/Dependencies.cmake)

# Collect all source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.hpp")

add_executable(masquerade_ball ${SOURCES} ${HEADERS})

target_include_directories(masquerade_ball PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(masquerade_ball PRIVATE
    ftxui::component
    ftxui::dom
    ftxui::screen
    box2d
    stb_image
)

if(ENABLE_GAMEPAD)
    target_link_libraries(masquerade_ball PRIVATE gamepad)
    target_compile_definitions(masquerade_ball PRIVATE HAS_GAMEPAD)
endif()

# Copy assets to build directory (for development)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Installation rules
include(GNUInstallDirs)

install(TARGETS masquerade_ball
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY assets/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/masquerade-ball
    FILES_MATCHING
        PATTERN "*.txt"
        PATTERN "*.png"
)

install(FILES masquerade-ball.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES LICENSE
    DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/${PROJECT_NAME}
)
