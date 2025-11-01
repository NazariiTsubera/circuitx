//
// Created by Nazarii on 11/1/25.
//

#ifndef UISERVICE_H
#define UISERVICE_H
#include <functional>

#include "Visualizer.h"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"


enum class ComponentType {
    Resistor,
    Capacitor,
    ISource,
    VSource
};

struct PaletteComponent {
    ComponentType type;
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

    //callbacks
    std::function<void(const sf::Vector2f& newSize)> resizeCallback;
public:
    UiService(sf::RenderWindow& window, const Visualizer& visualizer, AssetManager& assetManager);
    ~UiService();
    void drawUI();

    void setResizeCallback(std::function<void(const sf::Vector2f& newSize)>&& callback) { resizeCallback = std::move(callback); }
    sf::Vector2f getCanvasSize() const { return canvasSize; }
private:
    void drawCanvas();
    void drawPalette();

    float computePixelScale() const;
};



#endif //UISERVICE_H
