//
// Created by Nazarii on 10/30/25.
//

#ifndef APPLICATION_H
#define APPLICATION_H
#include <circuitx/circuit.hpp>
#include <imgui.h>

#include "SFML/Graphics/RenderWindow.hpp"

struct AppContext;

class Application {
private:
    sf::RenderWindow window;
    AppContext* context;
public:
    Application();
    ~Application();
    void run();
private:
    void handleEvents();
    void render();
    void configureHiDPISupport();
    float computePixelScale() const;
};


#endif //APPLICATION_H
