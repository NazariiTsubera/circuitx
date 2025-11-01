//
// Created by Nazarii on 10/31/25.
//

#include "Visualizer.h"

#include <imgui.h>
#include <SFML/Graphics/Vertex.hpp>

#include "SFML/Graphics/CircleShape.hpp"



struct VisualizerContext {
    sf::Texture capacitor;
    sf::Texture resistor;
    sf::Texture node;
    sf::Texture vsource;
    sf::Texture isource;
};


Visualizer::Visualizer()
    :  layout{}, renderQueue()
{
    context = new VisualizerContext();
}

Visualizer::~Visualizer() {
    delete context;
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

    for (auto& drawable : renderQueue) {
        target.draw(*drawable, sf::RenderStates::Default);
    }
}
