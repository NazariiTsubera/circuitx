cmake_minimum_required(VERSION 3.30)
project(circuitx)

set(CMAKE_CXX_STANDARD 26)

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE CACHE BOOL "Skip FetchContent updates" FORCE)

if (NOT TARGET sfml-system)
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

    foreach(component IN ITEMS Graphics Window System Audio Network)
        string(TOLOWER "${component}" component_lc)
        if (TARGET sfml-${component_lc} AND NOT TARGET SFML::${component})
            add_library(SFML::${component} ALIAS sfml-${component_lc})
        endif ()
    endforeach()
endif ()

if (NOT TARGET ImGui-SFML::ImGui-SFML)
    set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
    set(IMGUI_SFML_USE_SFML_CONFIG OFF CACHE BOOL "" FORCE)
    set(IMGUI_SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FOR1CE)
    set(IMGUI_SFML_BUILD_TESTS OFF CACHE BOOL "" FORCE)

    # imgui-sfml v2.6 targets Dear ImGui 1.89.2; keep versions in sync unless both are upgraded.
    FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.90.6-docking
    )
    FetchContent_MakeAvailable(imgui)
    set(IMGUI_DIR "${imgui_SOURCE_DIR}" CACHE PATH "Dear ImGui root for ImGui-SFML" FORCE)

    FetchContent_Declare(
            imgui-sfml
            GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
            GIT_TAG v2.6
    )
    FetchContent_MakeAvailable(imgui-sfml)
endif ()

if (NOT TARGET nlohmann_json::nlohmann_json)
    set(JSON_BuildTests OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif ()

if (NOT TARGET Eigen3::Eigen)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(EIGEN_BUILD_DOC OFF CACHE BOOL "" FORCE)
    set(EIGEN_BUILD_TESTING OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
            eigen
            GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
            GIT_TAG 3.4.0
    )
    FetchContent_MakeAvailable(eigen)
endif ()

add_executable(app
        app/main.cpp
        app/Application.cpp
        app/Application.h
        app/services/Visualizer.cpp
        app/services/Visualizer.h
        app/services/UiService.cpp
        app/services/UiService.h
        app/helpers/GridRenderer.hpp
        app/helpers/AssetManager.hpp
        app/services/CircuitService.cpp
        app/services/CircuitService.h
        app/helpers/WireTool.hpp
        app/helpers/CoordinateTool.hpp
        app/services/CircuitView.cpp
        app/services/CircuitView.h
        app/services/CircuitController.cpp
        app/services/CircuitController.h
)

add_library(circuitx STATIC
        solver/src/circuit.cpp
)

target_include_directories(circuitx PUBLIC solver/include/)

target_link_libraries(app PUBLIC circuitx)
target_link_libraries(circuitx PUBLIC nlohmann_json::nlohmann_json Eigen3::Eigen)
target_link_libraries(app PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)
target_link_libraries(app PRIVATE ImGui-SFML::ImGui-SFML)
