//
// Created by Nazarii on 10/31/25.
//

#include "Visualizer.h"

#include <imgui.h>
#include <SFML/Graphics/Vertex.hpp>

#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

Visualizer::Visualizer(const AssetManager& assetManager)
    :  layout{}, renderQueue(), assetManager(assetManager)
{}

Visualizer::Visualizer(const sf::View& view, const AssetManager& assetManager) : Visualizer(assetManager) {
    update(view);
}

Visualizer::~Visualizer() {}

void Visualizer::update(const sf::View &view) {
    gridRenderer.update(view);
}


void Visualizer::buildNodes(float r) {
    for (VisualNode node : layout.nodes) {
        std::unique_ptr<sf::CircleShape> circle = std::make_unique<sf::CircleShape>();
        circle->setRadius(r);
        circle->setOrigin(node.position);
        renderQueue.push_back(std::move(circle));
    }
}

void Visualizer::buildElements() {

}



void Visualizer::build(circuitx::Circuit circuit) {
    std::vector nodes = circuit.getNodes();
    std::vector elements = circuit.getElements();


}

void Visualizer::drawWire(sf::RenderTarget& target, const WirePreview& preview) const {

    // sf::VertexArray horizontal(sf::Lines);
    // horizontal.append(sf::Vertex(sf::Vector2f(preview.origin.x, preview.origin.y), sf::Color::Black));
    // horizontal.append(sf::Vertex(sf::Vector2f(preview.destination.x, preview.origin.y), sf::Color::Black));

    // sf::VertexArray vertical(sf::Lines);
    // vertical.append(sf::Vertex(sf::Vector2f(preview.destination.x, preview.origin.y), sf::Color::Black));
    // vertical.append(sf::Vertex(sf::Vector2f(preview.destination.x, preview.destination.y), sf::Color::Black));

    auto delta = preview.destination - preview.origin;

    sf::RectangleShape horizontal;
    horizontal.setFillColor(sf::Color::Red);

    sf::RectangleShape vertical;
    vertical.setFillColor(sf::Color::Red);

    if (std::abs(delta.x) > std::abs(delta.y)) {
        horizontal.setPosition(sf::Vector2f(preview.origin.x, preview.origin.y));
        horizontal.setSize(sf::Vector2f(preview.destination.x - preview.origin.x, 5.f));
        vertical.setPosition(sf::Vector2f(preview.destination.x, preview.origin.y));
        vertical.setSize(sf::Vector2f(5.f, preview.destination.y - preview.origin.y));
    }
    else {
        vertical.setPosition(sf::Vector2f(preview.origin.x, preview.origin.y));
        vertical.setSize(sf::Vector2f(5.f, preview.destination.y - preview.origin.y));

        horizontal.setPosition(sf::Vector2f(preview.origin.x, preview.destination.y));
        horizontal.setSize(sf::Vector2f(preview.destination.x - preview.origin.x, 5.f));
    }




    target.draw(vertical);
    target.draw(horizontal);


}

void Visualizer::draw(sf::RenderTarget& target, std::optional<WirePreview> preview) const {
    target.clear(sf::Color::White);

    gridRenderer.draw(target, sf::RenderStates::Default);

    if (preview.has_value()) {
        drawWire(target, preview.value());
    }

    for (auto& drawable : renderQueue) {
        target.draw(*drawable, sf::RenderStates::Default);
    }
}

