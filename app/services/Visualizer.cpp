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
    : gridRenderer(), gridSettings(gridSettings), circuitView(circuitView) {}

Visualizer::Visualizer(const sf::View& view, const GridSettings& gridSettings, const CircuitView& circuitView)
    : Visualizer(gridSettings, circuitView) {
    update(view);
}

Visualizer::~Visualizer() = default;

void Visualizer::update(const sf::View& view) {
    gridRenderer.update(view, gridSettings);
}

void Visualizer::draw(sf::RenderTarget& target, std::optional<WirePreview> preview) const {
    target.clear(sf::Color::White);

    gridRenderer.draw(target, sf::RenderStates::Default);

    for (const auto& wire : circuitView.getWires()) {
        const auto startOpt = circuitView.getNodePosition(wire.startNode);
        const auto endOpt = circuitView.getNodePosition(wire.endNode);
        if (!startOpt.has_value() || !endOpt.has_value()) {
            continue;
        }
        drawWire(target, *startOpt, *endOpt, sf::Color(80, 80, 90));
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
        circle.setFillColor(sf::Color(30, 144, 255));
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
    body.setSize({32.f, 14.f});
    body.setOrigin(body.getSize().x * 0.5f, body.getSize().y * 0.5f);
    body.setPosition(component.position);

    sf::Color fillColor;
    switch (component.type) {
        case ComponentType::Resistor:
            fillColor = sf::Color(210, 180, 140);
            break;
        case ComponentType::Capacitor:
            fillColor = sf::Color(100, 149, 237);
            break;
        case ComponentType::ISource:
            fillColor = sf::Color(144, 238, 144);
            break;
        case ComponentType::VSource:
            fillColor = sf::Color(255, 160, 122);
            break;
        case ComponentType::Wire:
            return;
    }
    body.setFillColor(fillColor);
    body.setOutlineThickness(1.f);
    body.setOutlineColor(sf::Color::Black);

    body.setRotation(rotationStepsToDegrees(component.rotationSteps));

    target.draw(body);
}
