//
// Created by Nazarii on 10/31/25.
//

#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <optional>
#include <vector>

#include <circuitx/circuit.hpp>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

#include "../helpers/GridRenderer.hpp"
#include "../helpers/WireTool.hpp"
#include "CircuitView.h"

class Visualizer {
public:
    Visualizer(const GridSettings& grid_settings, const CircuitView& circuit_view);
    Visualizer(const sf::View& view, const GridSettings& grid_settings, const CircuitView& circuit_view);
    ~Visualizer();

    void update(const sf::View& view);
    void draw(sf::RenderTarget& target, std::optional<WirePreview> preview = std::nullopt) const;

private:
    void drawPreviewWire(sf::RenderTarget& target, const WirePreview& preview) const;
    void drawWire(sf::RenderTarget& target, sf::Vector2f start, sf::Vector2f end, const sf::Color& color) const;

private:
    GridRenderer gridRenderer;
    GridSettings gridSettings;
    const CircuitView& circuitView;
};

#endif //VISUALIZER_H
