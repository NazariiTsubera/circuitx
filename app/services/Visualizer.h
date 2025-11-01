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


struct VisualNode {
    unsigned int id;
    sf::Vector2<float> position;
};

struct VisuaElement {
    sf::Image image;
    sf::Text label;
};


struct Layout {
    std::vector<VisualNode> nodes;
    std::vector<VisuaElement> elements;
};

struct VisualizerContext;

class Visualizer {
private:
    Layout layout;
    std::vector<std::unique_ptr<sf::Drawable>> renderQueue;
    VisualizerContext *context;
public:
    Visualizer();
    ~Visualizer();

    void build(circuitx::Circuit circuit);
    void draw(sf::RenderTarget& target) const;
private:
    void buildNodes(float r);
    void buildElements();
};



#endif //VISUALIZER_H
