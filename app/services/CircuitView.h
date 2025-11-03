//
// Created by Nazarii on 11/1/25.
//

#ifndef CIRCUITVIEW_H
#define CIRCUITVIEW_H
#include <cmath>
#include <optional>
#include <unordered_map>
#include <vector>

#include "SFML/System/Vector2.hpp"
#include "ComponentType.h"

struct WireView {
    unsigned int startNode;
    unsigned int endNode;
};

struct ComponentView {
    unsigned int id;
    ComponentType type;
    sf::Vector2f position;
};

class CircuitView {
public:
    explicit CircuitView(float snapRadius = 20.f)
        : snapRadius(snapRadius), snapRadiusSquared(snapRadius * snapRadius) {}

    void recordNode(unsigned int nodeId, sf::Vector2f position) {
        nodePositions[nodeId] = position;
    }

    void recordComponent(unsigned int componentId, ComponentType type, sf::Vector2f position) {
        components[componentId] = ComponentView{componentId, type, position};
    }

    std::optional<sf::Vector2f> getNodePosition(unsigned int nodeId) const {
        if (auto it = nodePositions.find(nodeId); it != nodePositions.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::optional<unsigned int> getNodeNear(sf::Vector2f position) const {
        for (const auto& [nodeId, storedPos] : nodePositions) {
            if (isInProximity(storedPos, position)) {
                return nodeId;
            }
        }
        return std::nullopt;
    }

    void recordWire(unsigned int startNode, unsigned int endNode) {
        wires.push_back({startNode, endNode});
    }

    const std::unordered_map<unsigned int, sf::Vector2f>& getNodes() const { return nodePositions; }
    const std::vector<WireView>& getWires() const { return wires; }
    const std::unordered_map<unsigned int, ComponentView>& getComponents() const { return components; }

private:
    bool isInProximity(sf::Vector2f a, sf::Vector2f b) const {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        return (dx * dx + dy * dy) <= snapRadiusSquared;
    }

    float snapRadius;
    float snapRadiusSquared;
    std::unordered_map<unsigned int, sf::Vector2f> nodePositions;
    std::vector<WireView> wires;
    std::unordered_map<unsigned int, ComponentView> components;
};


#endif //CIRCUITVIEW_H
