//
// Created by Nazarii on 10/31/25.
//

#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <queue>
#include <vector>

#include <circuitx/circuit.hpp>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

#include "CircuitView.h"
#include "../helpers/GridRenderer.hpp"
#include "../helpers/WireTool.hpp"

#include "SFML/Graphics/CircleShape.hpp"


class AssetManager;

struct VisualizerContext;

class Visualizer {
private:
    std::vector<std::unique_ptr<sf::Drawable>> renderQueue;
    GridRenderer gridRenderer;
    const AssetManager& assetManager;
    const CircuitView& circuitView;
    GridSettings gridSettings;
public:
    Visualizer(const AssetManager& asset_manager, const GridSettings& grid_settings, const CircuitView& circuit_view);
    Visualizer(const sf::View &view, const AssetManager& asset_manager, const GridSettings& grid_settings, const CircuitView& circuit_view);
    ~Visualizer();

    void update(const sf::View &view);
    void build(const CircuitView& circuit);
    void draw(sf::RenderTarget& target, std::optional<WirePreview> preview = std::nullopt) const;

private:
    void drawWire(sf::RenderTarget& target, const WirePreview& preview) const;
    void buildNodes(float r);
    void buildElements();
};



#endif //VISUALIZER_H
