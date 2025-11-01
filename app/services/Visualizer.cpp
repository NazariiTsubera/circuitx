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


void Visualizer::draw(sf::RenderTarget& target) const {
    target.clear(sf::Color::White);

    gridRenderer.draw(target, sf::RenderStates::Default);

    for (auto& drawable : renderQueue) {
        target.draw(*drawable, sf::RenderStates::Default);
    }
}

