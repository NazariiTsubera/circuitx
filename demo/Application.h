//
// Created by Nazarii on 10/30/25.
//

#ifndef APPLICATION_H
#define APPLICATION_H
#include <circuitx/circuit.hpp>

#include "SFML/Graphics/RenderWindow.hpp"


class Application {
private:
    circuitx::Circuit* circuit;
    sf::RenderWindow window;
public:
    Application();
    ~Application();
    void run();
};

#endif //APPLICATION_H
