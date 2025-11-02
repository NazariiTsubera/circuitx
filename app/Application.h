//
// Created by Nazarii on 10/30/25.
//

#ifndef APPLICATION_H
#define APPLICATION_H
#include <circuitx/circuit.hpp>
#include <imgui.h>

#include "services/UiService.h"
#include "services/Visualizer.h"
#include "SFML/Graphics/RenderWindow.hpp"
#include "helpers/AssetManager.hpp"

struct AppContext;

class Application {
private:
    sf::RenderWindow window;
    sf::Clock clock;
    circuitx::Circuit circuit;
    AssetManager assetManager;
    Visualizer visualizer;
    GridSettings gridSettings {30};
    CoordinateTool gridTool;
    UiService uiService;
public:
    Application();
    ~Application();
    void run();
private:
    void handleEvents();
    void render();
};


#endif //APPLICATION_H
