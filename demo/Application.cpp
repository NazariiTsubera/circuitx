//
// Created by Nazarii on 10/30/25.
//

#include "Application.h"

#include <iostream>

#include "SFML/Window/Event.hpp"

Application::Application()
    :
    window(sf::RenderWindow(sf::VideoMode::getDesktopMode(),
        "Circuitx", sf::Style::Titlebar | sf::Style::Default)),
    circuit{}
{

}

Application::~Application() {
}

void Application::run() {

    window.setFramerateLimit(60);
    window.setActive(true);

    while (window.isOpen()) {

        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::EventType::Closed) {
                window.close();
            }

            if (event.type == sf::Event::EventType::KeyPressed) {
                std::cout << event.key.code << std::endl;
            }
        }
    }
}
