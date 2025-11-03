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

struct WireView {
    unsigned int startNode;
    unsigned int endNode;
};

class CircuitView {
public:
    explicit CircuitView(float snapRadius = 20.f)
        : snapRadius(snapRadius), snapRadiusSquared(snapRadius * snapRadius) {}

    void recordNode(unsigned int nodeId, sf::Vector2f position) {
        nodePositions[nodeId] = position;
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
};


#endif //CIRCUITVIEW_H
