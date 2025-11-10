//
// Created by Nazarii on 10/30/25.
//

#include "Application.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>

#include <filesystem>
#include <fstream>
#include <vector>

namespace {
std::string locateResourceRoot() {
    namespace fs = std::filesystem;
    const fs::path cwd = fs::current_path();
    const fs::path executableDir = [] {
        try {
            return fs::canonical(fs::path(std::filesystem::current_path()));
        } catch (...) {
            return std::filesystem::current_path();
        }
    }();

    std::vector<fs::path> candidates = {
        cwd / "res",
        cwd / "../res",
        cwd / "../../res",
        executableDir / "res",
        executableDir.parent_path() / "res"
    };

    for (const auto& candidate : candidates) {
        if (fs::exists(candidate / "logo.png")) {
            fs::path normalized = fs::weakly_canonical(candidate);
            return (normalized.string() + "/");
        }
    }

    return (cwd / "res").string() + "/";
}

const char* kDefaultImguiLayout = R"INI([Window][DockHost]
Pos=0,45
Size=1920,887
Collapsed=0

[Window][Debug##Default]
Pos=60,60
Size=400,400
Collapsed=0

[Window][Palette]
Pos=901,45
Size=1019,552
Collapsed=0
DockId=0x00000003,0

[Window][Canvas]
Pos=0,45
Size=899,887
Collapsed=0
DockId=0x00000001,0

[Window][Topology]
Pos=901,599
Size=1019,333
Collapsed=0
DockId=0x00000004,1

[Window][Control panel]
Pos=901,599
Size=1019,333
Collapsed=0
DockId=0x00000004,0

[Docking][Data]
DockSpace     ID=0x45A01BCF Window=0x9BD87705 Pos=0,45 Size=1920,887 Split=X
  DockNode    ID=0x00000001 Parent=0x45A01BCF SizeRef=899,887 Selected=0x429E880E
  DockNode    ID=0x00000002 Parent=0x45A01BCF SizeRef=1019,887 Split=Y
    DockNode  ID=0x00000003 Parent=0x00000002 SizeRef=1319,552 CentralNode=1 Selected=0x7E84447F
    DockNode  ID=0x00000004 Parent=0x00000002 SizeRef=1319,333 Selected=0x22F9F188
)INI";

std::string ensureConfigIni() {
    namespace fs = std::filesystem;
    const fs::path cwd = fs::current_path();
    std::vector<fs::path> candidates = {
        cwd / "config" / "circuitx_imgui.ini",
        cwd / "../config" / "circuitx_imgui.ini",
        cwd / "../../config" / "circuitx_imgui.ini"
    };

    for (const auto& candidate : candidates) {
        if (fs::exists(candidate)) {
            return fs::weakly_canonical(candidate).string();
        }
    }

    fs::path fallback = cwd / "config" / "circuitx_imgui.ini";
    try {
        fs::create_directories(fallback.parent_path());
        std::ofstream out(fallback);
        out << kDefaultImguiLayout;
    } catch (...) {
    }
    try {
        return fs::weakly_canonical(fallback).string();
    } catch (...) {
        return fallback.string();
    }
}
}

Application::Application()
    :
      window(sf::RenderWindow(sf::VideoMode::getDesktopMode(), "Circuitx", sf::Style::Titlebar | sf::Style::Default)),
      gridTool(gridSettings),
      stateService(State::Edit),
      circuitController(circuitService, circuitView, gridTool),
      assetManager(locateResourceRoot()),
      imguiConfigPath(ensureConfigIni()),
      visualizer(gridSettings, circuitView),
      uiService(window,
                visualizer,
                assetManager,
                gridTool,
                circuitController,
                stateService,
                imguiConfigPath)
{
    sf::Image icon;
    const std::string logoPath = assetManager.getRoot() + "logo.png";
    if (icon.loadFromFile(logoPath)) {
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    }

    stateService.addCallback([this](State prev, State next) {
        if (next == State::Play)
            circuitController.simulate();
    });
    uiService.setResizeCallback([this](const sf::Vector2f& newSize) {
        sf::View view({newSize.x * 0.5f, newSize.y * 0.5f}, newSize);
        visualizer.update(view);
    });
}

Application::~Application() = default;




void Application::handleEvents() {
    sf::Event event{};

    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(window, event);

        if (event.type == sf::Event::EventType::Closed) {
            window.close();
        }

        if (event.type == sf::Event::EventType::KeyPressed) {
            std::cout << event.key.code << std::endl;
        }
    }
}


void Application::render() {
    ImGui::SFML::Update(window, clock.restart());

    window.clear();
    uiService.drawUI();
    ImGui::SFML::Render(window);
    window.display();
}


void Application::run() {

    window.setFramerateLimit(60);

    while (window.isOpen()) {
        handleEvents();
        render();
    }

    ImGui::SFML::Shutdown();
}
