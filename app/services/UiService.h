//
// Created by Nazarii on 11/1/25.
//

#ifndef UISERVICE_H
#define UISERVICE_H
#include <functional>

#include "CircuitController.h"
#include "CircuitService.h"
#include "Visualizer.h"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "CircuitService.h"



struct PaletteComponent {
    ComponentType type;
    std::string name;
    sf::Texture texture;
};

class UiService {
private:
    sf::RenderWindow& window;
    sf::RenderTexture canvasTexture;
    const Visualizer& visualizer;
    AssetManager& assetManager;
    sf::Vector2f canvasSize;
    std::vector<PaletteComponent> components;
    const CoordinateTool& gridTool;
    WireTool wireTool;
    CircuitController& circuitController;


    //callbacks
    std::function<void(const sf::Vector2f& newSize)> resizeCallback;
public:
    UiService(sf::RenderWindow& window, const Visualizer& visualizer, AssetManager& assetManager, const CoordinateTool& gridTool, CircuitController& circuitController);
    ~UiService();
    void drawUI();

    void setResizeCallback(std::function<void(const sf::Vector2f& newSize)>&& callback) { resizeCallback = std::move(callback); }
    sf::Vector2f getCanvasSize() const { return canvasSize; }
private:
    void drawCanvas();
    void drawPalette();
    void drawTopology();

    float computePixelScale() const;
};



#endif //UISERVICE_H
