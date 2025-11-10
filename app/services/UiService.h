//
// Created by Nazarii on 11/1/25.
//

#ifndef UISERVICE_H
#define UISERVICE_H
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "CircuitController.h"
#include "StateService.h"
#include "Visualizer.h"
#include "../helpers/AssetManager.hpp"
#include "../ui/CanvasPanel.h"
#include "../ui/UiState.h"
#include "../ui/panels/ControlPanel.h"
#include "../ui/panels/NavPanel.h"
#include "../ui/panels/PalettePanel.h"
#include "../ui/panels/PropertiesPanel.h"
#include "../ui/panels/SettingsPanel.h"
#include "../ui/panels/SimulationPanel.h"
#include "../ui/panels/ToolboxPanel.h"
#include "../ui/panels/TopologyPanel.h"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

class UiService {
private:
    sf::RenderWindow& window;
    sf::RenderTexture canvasTexture;
    Visualizer& visualizer;
    AssetManager& assetManager;
    sf::Vector2f canvasSize;
    const CoordinateTool& gridTool;
    WireTool wireTool;
    CircuitController& circuitController;
    StateService& stateService;
    //callbacks
    std::function<void(const sf::Vector2f& newSize)> resizeCallback;
    UiState uiState;
    CanvasPanel canvasPanel;
    NavPanel navPanel;
    PalettePanel palettePanel;
    ToolboxPanel toolboxPanel;
    ControlPanel controlPanel;
    SimulationPanel simulationPanel;
    SettingsPanel settingsPanel;
    PropertiesPanel propertiesPanel;
    TopologyPanel topologyPanel;
    std::string imguiConfigPath;
public:
    UiService(sf::RenderWindow& window, Visualizer& visualizer,
            AssetManager& assetManager, const CoordinateTool& gridTool,
            CircuitController& circuitController, StateService& stateService,
            const std::string& imguiConfigPath);
    ~UiService();
    void drawUI();

    void setResizeCallback(std::function<void(const sf::Vector2f& newSize)>&& callback) { resizeCallback = std::move(callback); }
    sf::Vector2f getCanvasSize() const { return canvasSize; }
private:
    float computePixelScale() const;

    void applyTheme(UiTheme theme);
};



#endif //UISERVICE_H
