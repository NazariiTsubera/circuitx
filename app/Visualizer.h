//
// Created by Nazarii on 10/31/25.
//

#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <vector>

#include <circuitx/circuit.hpp>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>


struct VisualNode {
    unsigned int id;
    sf::Vector2<float> position;
};

struct VisuaElement {
    sf::Image image;
    sf::Text label;
};

class Visualizer {
private:
    std::vector<VisualNode> nodes;
    std::vector<VisuaElement> elements;

    sf::VertexArray nodesVA;
    sf::VertexArray elementsVA;
public:
    Visualizer() : nodes(), elements() {};
    ~Visualizer() = default;

    void build(circuitx::Circuit circuit); // must call before all draw calls
    void draw(sf::RenderTarget& target) const;
};



#endif //VISUALIZER_H
