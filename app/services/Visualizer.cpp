//
// Created by Nazarii on 10/31/25.
//

#include "Visualizer.h"

#include <cmath>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include "SFML/Graphics/Color.hpp"

Visualizer::Visualizer(const GridSettings& gridSettings, const CircuitView& circuitView)
    : gridRenderer(), gridSettings(gridSettings), circuitView(circuitView) {
    setTheme(VisualizerTheme::Dark);
}

Visualizer::Visualizer(const sf::View& view, const GridSettings& gridSettings, const CircuitView& circuitView)
    : Visualizer(gridSettings, circuitView) {
    update(view);
}

Visualizer::~Visualizer() = default;

void Visualizer::update(const sf::View& view) {
    cachedView = view;
    hasCachedView = true;
    gridRenderer.update(view, gridSettings);
}

void Visualizer::draw(sf::RenderTarget& target, std::optional<WirePreview> preview) const {
    target.clear(backgroundColor);

    gridRenderer.draw(target, sf::RenderStates::Default);

    for (const auto& wire : circuitView.getWires()) {
        const auto startOpt = circuitView.getNodePosition(wire.startNode);
        const auto endOpt = circuitView.getNodePosition(wire.endNode);
        if (!startOpt.has_value() || !endOpt.has_value()) {
            continue;
        }
        drawWire(target, *startOpt, *endOpt, wireColor);
    }

    for (const auto& [componentId, component] : circuitView.getComponents()) {
        drawComponent(target, component);
    }

    if (preview.has_value()) {
        drawPreviewWire(target, preview.value());
    }

    for (const auto& [nodeId, position] : circuitView.getNodes()) {
        sf::CircleShape circle;
        circle.setRadius(4.f);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setFillColor(nodeColor);
        circle.setPosition(position);
        target.draw(circle);
    }
}

void Visualizer::drawPreviewWire(sf::RenderTarget& target, const WirePreview& preview) const {
    drawWire(target, preview.origin, preview.destination, sf::Color::Red);
}

void Visualizer::drawWire(sf::RenderTarget& target, sf::Vector2f start, sf::Vector2f end, const sf::Color& color) const {
    constexpr float thickness = 4.f;

    const sf::Vector2f delta = end - start;
    const bool horizontalFirst = std::abs(delta.x) >= std::abs(delta.y);
    const sf::Vector2f elbow = horizontalFirst ? sf::Vector2f(end.x, start.y) : sf::Vector2f(start.x, end.y);

    auto makeSegment = [&](sf::Vector2f a, sf::Vector2f b) {
        sf::RectangleShape segment;
        segment.setFillColor(color);

        const float width = b.x - a.x;
        const float height = b.y - a.y;

        if (std::abs(width) >= std::abs(height)) {
            const float length = std::abs(width);
            segment.setSize({length, thickness});
            segment.setOrigin(0.f, thickness * 0.5f);
            segment.setPosition(width >= 0.f ? a : sf::Vector2f(b.x, a.y));
        } else {
            const float length = std::abs(height);
            segment.setSize({thickness, length});
            segment.setOrigin(thickness * 0.5f, 0.f);
            segment.setPosition(height >= 0.f ? a : sf::Vector2f(a.x, b.y));
        }

        return segment;
    };

    sf::RectangleShape firstSegment = makeSegment(start, elbow);
    sf::RectangleShape secondSegment = makeSegment(elbow, end);

    target.draw(firstSegment);
    target.draw(secondSegment);
}

void Visualizer::drawComponent(sf::RenderTarget& target, const ComponentView& component) const {
    sf::RectangleShape body;
    body.setSize({34.f, 14.f});
    body.setOrigin(body.getSize().x * 0.5f, body.getSize().y * 0.5f);
    body.setPosition(component.position);

    sf::Color fillColor;
    switch (component.type) {
        case ComponentType::Resistor:
            fillColor = (currentTheme == VisualizerTheme::Dark)
                        ? sf::Color(241, 168, 94)
                        : sf::Color(210, 142, 74);
            break;
        case ComponentType::Capacitor:
            fillColor = (currentTheme == VisualizerTheme::Dark)
                        ? sf::Color(104, 169, 255)
                        : sf::Color(68, 134, 230);
            break;
        case ComponentType::ISource:
            fillColor = (currentTheme == VisualizerTheme::Dark)
                        ? sf::Color(86, 210, 171)
                        : sf::Color(58, 176, 136);
            break;
        case ComponentType::VSource:
            fillColor = (currentTheme == VisualizerTheme::Dark)
                        ? sf::Color(252, 120, 120)
                        : sf::Color(235, 98, 98);
            break;
        case ComponentType::Wire:
            return;
    }
    body.setFillColor(fillColor);
    body.setOutlineThickness(1.4f);
    body.setOutlineColor(outlineColor);

    body.setRotation(rotationStepsToDegrees(component.rotationSteps));

    target.draw(body);
}

void Visualizer::setTheme(VisualizerTheme theme) {
    currentTheme = theme;
    if (theme == VisualizerTheme::Dark) {
        backgroundColor = sf::Color(22, 24, 31);
        wireColor = sf::Color(108, 115, 132);
        nodeColor = sf::Color(93, 193, 255);
        outlineColor = sf::Color(16, 18, 22);
        gridRenderer.major = sf::Color(60, 66, 80, 200);
        gridRenderer.minor = sf::Color(38, 42, 54, 140);
    } else {
        backgroundColor = sf::Color(247, 249, 253);
        wireColor = sf::Color(140, 146, 160);
        nodeColor = sf::Color(74, 117, 209);
        outlineColor = sf::Color(214, 217, 224);
        gridRenderer.major = sf::Color(207, 212, 224, 220);
        gridRenderer.minor = sf::Color(227, 230, 238, 160);
    }
    if (hasCachedView) {
        gridRenderer.update(cachedView, gridSettings);
    }
}
