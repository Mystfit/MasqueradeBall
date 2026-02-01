cmake_minimum_required(VERSION 3.20)
project(MasqueradeBall VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fetch dependencies
include(cmake/Dependencies.cmake)

# Collect all source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.hpp")

add_executable(masquerade_ball ${SOURCES} ${HEADERS})

target_include_directories(masquerade_ball PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

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

# Copy assets to build directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
