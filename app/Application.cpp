//
// Created by Nazarii on 10/30/25.
//

#include "Application.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-SFML.h>

#include <filesystem>

Application::Application()
    : window(sf::RenderWindow(sf::VideoMode::getDesktopMode(), "Circuitx", sf::Style::Titlebar | sf::Style::Default)),
      clock(),
      gridSettings{20},
      gridTool(gridSettings),
      circuitService(),
      circuitView(),
      circuitController(circuitService, circuitView, gridTool),
      assetManager("../res/"),
      visualizer(gridSettings, circuitView),
      uiService(window,
                visualizer,
                assetManager,
                gridTool,
                circuitController,
                stateService) {

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
