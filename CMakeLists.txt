cmake_minimum_required(VERSION 3.30)
project(circuitx)

set(CMAKE_CXX_STANDARD 26)

include(FetchContent)

# Cache options ahead of time so FetchContent reuses them on reconfigure.
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE CACHE BOOL "Skip FetchContent updates" FORCE)

if (NOT TARGET sfml-system)
    # SFML provides windowing/audio/graphics utilities used by the demo app.
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
    set(SFML_USE_SYSTEM_DEPS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
            sfml
            GIT_REPOSITORY https://github.com/SFML/SFML.git
            GIT_TAG 2.6.1
    )
    FetchContent_MakeAvailable(sfml)
endif ()


# Avoid refetching dependencies after initial download.
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE CACHE BOOL "Skip FetchContent updates" FORCE)

if (NOT TARGET sfml-system)
    # Configure SFML FetchContent (2.6.1) without docs/examples.
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK ON CACHE BOOL "" FORCE)
    set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
    set(SFML_USE_SYSTEM_DEPS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
            sfml
            GIT_REPOSITORY https://github.com/SFML/SFML.git
            GIT_TAG 2.6.1
    )
    FetchContent_MakeAvailable(sfml)
endif ()

if (NOT TARGET ImGui-SFML::ImGui-SFML)
    set(IMGUI_SFML_FIND_SFML ON CACHE BOOL "" FORCE)
    set(IMGUI_SFML_USE_SFML_CONFIG OFF CACHE BOOL "" FORCE)
    set(IMGUI_SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(IMGUI_SFML_BUILD_TESTS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.91.1
    )

    FetchContent_Declare(
            imgui-sfml
            GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
            GIT_TAG v2.6
    )

    FetchContent_MakeAvailable(imgui imgui-sfml)
endif ()



add_executable(demo demo/main.cpp
        demo/Application.cpp
        demo/Application.h)

add_library(circuitx STATIC
        src/circuit.cpp
)

target_include_directories(circuitx PUBLIC include/)

target_link_libraries(demo PUBLIC circuitx)
target_link_libraries(demo PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)
target_link_libraries(demo PRIVATE ImGui-SFML::ImGui-SFML)
