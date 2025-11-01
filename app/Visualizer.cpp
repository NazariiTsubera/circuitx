//
// Created by Nazarii on 10/31/25.
//

#include "Visualizer.h"

#include <SFML/Graphics/Vertex.hpp>


void Visualizer::build(circuitx::Circuit circuit) {
    std::vector nodes = circuit.getNodes();
    std::vector elements = circuit.getElements();


    int side = ceil(sqrt(nodes.size()));


    nodesVA.clear();
    nodesVA.setPrimitiveType(sf::Points);

    float dim = 1000.f; // unhardcode

    for (int i = 0; i < side; i++) {
        for (int j = 0; j < side; j++) {
            sf::Vector2<float> position(i * dim, j * dim);
            sf::Color color = sf::Color::White;
            nodesVA.append(sf::Vertex(position, color));
        }
    }

}

void Visualizer::draw(sf::RenderTarget& target) const {
    target.draw(nodesVA, sf::RenderStates::Default);
}
