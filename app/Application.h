//
// Created by Nazarii on 10/30/25.
//

#ifndef APPLICATION_H
#define APPLICATION_H
#include <circuitx/circuit.hpp>
#include <string>
#include <imgui.h>

#include "services/CircuitController.h"
#include "services/UiService.h"
#include "services/Visualizer.h"
#include "SFML/Graphics/RenderWindow.hpp"
#include "helpers/AssetManager.hpp"
#include "helpers/CoordinateTool.hpp"

class Application {
private:
    sf::RenderWindow window;
    sf::Clock clock;
    GridSettings gridSettings {20};
    CoordinateTool gridTool;
    StateService stateService;
    CircuitService circuitService;
    CircuitView circuitView;
    CircuitController circuitController;
    AssetManager assetManager;
    std::string imguiConfigPath;
    Visualizer visualizer;
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
