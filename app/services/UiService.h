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
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

enum class UiTheme {
    Black,
    White
};

struct PaletteComponent {
    ComponentType type;
    std::string name;
    sf::Texture texture;
};

struct StateOptions {
    State state;
    std::string name;
    sf::Texture texture;
};


class UiService {
private:
    sf::RenderWindow& window;
    sf::RenderTexture canvasTexture;
    Visualizer& visualizer;
    AssetManager& assetManager;
    sf::Vector2f canvasSize;
    std::vector<PaletteComponent> components;
    std::vector<StateOptions> states;
    const CoordinateTool& gridTool;
    WireTool wireTool;
    CircuitController& circuitController;
    std::optional<sf::Vector2f> contextMenuPosition;
    StateService& stateService;
    //callbacks
    std::function<void(const sf::Vector2f& newSize)> resizeCallback;
public:
    UiService(sf::RenderWindow& window, Visualizer& visualizer,
            AssetManager& assetManager, const CoordinateTool& gridTool,
            CircuitController& circuitController, StateService& stateService);
    ~UiService();
    void drawUI();

    void setResizeCallback(std::function<void(const sf::Vector2f& newSize)>&& callback) { resizeCallback = std::move(callback); }
    sf::Vector2f getCanvasSize() const { return canvasSize; }
private:
    void drawCanvas();
    void drawPalette();
    void drawToolbox();
    void drawTopology();
    void drawControlPanel();
    void drawSimulation();
    void drawPropertiesWindow();

    float computePixelScale() const;
    std::optional<ComponentView> propertiesComponent;
    std::optional<ComponentView> contextMenuComponent;
    std::optional<WireView> contextMenuWire;
    std::optional<unsigned int> selectedComponentId;
    bool showPropertiesWindow = false;
    float propertiesValue = 0.f;
    std::string propertiesStatus;
    std::string toolboxStatus;
    int placementRotationSteps = 0;
    bool toolboxVisible = false;
    bool toolboxHovered = false;
    UiTheme currentTheme = UiTheme::Black;

    void applyTheme(UiTheme theme);
};



#endif //UISERVICE_H
