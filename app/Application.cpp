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
#include <imgui-SFML.h>

#include "Visualizer.h"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/Shape.hpp"

Application::Application()
    :
    window(sf::RenderWindow(sf::VideoMode::getDesktopMode(),
        "Circuitx", sf::Style::Titlebar | sf::Style::Default)),
    context{}
{

}

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



struct AppContext {
    sf::Clock clock;
    circuitx::Circuit circuit;
    Visualizer visualizer;

    AppContext() : visualizer(), clock(), circuit() {
        circuitx::Node n(1, "Node1");
        circuitx::Node n2(2, "Node2");
        circuitx::Node n3(3, "Node3");

        circuitx::Res res1(1, 2, 9);
        circuitx::Res cap1(1, 2, 9);

        circuit.addNode(n);
        circuit.addNode(n2);
        circuit.addNode(n3);
        circuit.addElement(res1);
        circuit.addElement(cap1);

        visualizer.build(circuit);

    }
};




void Application::render() {
    ImGui::SFML::Update(window, context->clock.restart());

    ImGui::Begin("CircuitX");
    ImGui::End();

    window.clear(sf::Color(30, 30, 40));
    context->visualizer.draw(window);
    ImGui::SFML::Render(window);
    window.display();
}


void Application::run() {

    window.setFramerateLimit(60);

    const bool imguiInitialized = ImGui::SFML::Init(window);

    if (!imguiInitialized) {
        throw std::runtime_error("Failed to initialize ImGui-SFML.");
    }

    configureHiDPISupport();
    context = new AppContext();

    while (window.isOpen()) {
        handleEvents();
        render();
    }

    ImGui::SFML::Shutdown();
}

void Application::configureHiDPISupport() {
    const float pixelScale = computePixelScale();
    if (pixelScale <= 1.0f) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig fontConfig;
    fontConfig.SizePixels = 13.0f * pixelScale;

    io.Fonts->Clear();
    io.Fonts->AddFontDefault(&fontConfig);
    io.DisplayFramebufferScale = ImVec2(pixelScale, pixelScale);

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(pixelScale);

    if (!ImGui::SFML::UpdateFontTexture()) {
        throw std::runtime_error("Failed to rebuild ImGui font texture for HiDPI scaling.");
    }
}

float Application::computePixelScale() const {
    const sf::Vector2u framebufferSize = window.getSize();
    const sf::Vector2f logicalSize = window.getView().getSize();

    if (logicalSize.x <= 0.0f || logicalSize.y <= 0.0f) {
        return 1.0f;
    }

    const float scaleX = static_cast<float>(framebufferSize.x) / logicalSize.x;
    const float scaleY = static_cast<float>(framebufferSize.y) / logicalSize.y;

    return std::max(scaleX, scaleY);
}


Application::~Application() {
    delete context;
}