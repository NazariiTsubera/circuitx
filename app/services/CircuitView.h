//
// Created by Nazarii on 11/1/25.
//

#ifndef CIRCUITVIEW_H
#define CIRCUITVIEW_H
#include <cmath>
#include <algorithm>
#include <optional>
#include <unordered_map>
#include <vector>

#include "SFML/System/Vector2.hpp"
#include "ComponentType.h"

struct ComponentView {
    unsigned int id;
    ComponentType type;
    sf::Vector2f position;
    unsigned int nodeA;
    unsigned int nodeB;
    int rotationSteps = 0;
};

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

    void recordComponent(unsigned int componentId, ComponentType type, sf::Vector2f position,
                         unsigned int nodeA, unsigned int nodeB, int rotationSteps = 0) {
        if (type == ComponentType::Wire) {
            wires.push_back({nodeA, nodeB});
            return;
        }
        components[componentId] = ComponentView{componentId, type, position, nodeA, nodeB, rotationSteps};
    }

    bool setComponentRotation(unsigned int componentId, int rotationSteps) {
        if (auto it = components.find(componentId); it != components.end()) {
            it->second.rotationSteps = rotationSteps;
            return true;
        }
        return false;
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

    void removeWireAt(std::size_t index) {
        if (index < wires.size()) {
            wires.erase(wires.begin() + static_cast<std::ptrdiff_t>(index));
        }
    }

    std::optional<WireView> removeWireAtPosition(sf::Vector2f position, float tolerance = 6.f) {
        if (auto index = getWireIndexAt(position, tolerance)) {
            WireView removed = wires[*index];
            wires.erase(wires.begin() + static_cast<std::ptrdiff_t>(*index));
            return removed;
        }
        return std::nullopt;
    }


    const std::unordered_map<unsigned int, sf::Vector2f>& getNodes() const { return nodePositions; }
    const std::vector<WireView>& getWires() const { return wires; }
    const std::unordered_map<unsigned int, ComponentView>& getComponents() const { return components; }
    std::optional<ComponentView> getComponent(unsigned int componentId) const {
        if (auto it = components.find(componentId); it != components.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    const WireView& getWire(std::size_t index) const { return wires.at(index); }
    std::size_t wireCount() const { return wires.size(); }

    bool hasSelectableAt(sf::Vector2f position) const {
        return hasComponentAt(position) || getWireIndexAt(position).has_value();
    }

    std::optional<ComponentView> removeComponentAt(sf::Vector2f position, float tolerance = 6.f) {
        const sf::Vector2f halfSize{16.f, 7.f};
        for (auto it = components.begin(); it != components.end(); ++it) {
            const sf::Vector2f delta{position.x - it->second.position.x, position.y - it->second.position.y};
            if (std::abs(delta.x) <= halfSize.x + tolerance && std::abs(delta.y) <= halfSize.y + tolerance) {
                ComponentView removed = it->second;
                components.erase(it);
                return removed;
            }
        }
        return std::nullopt;
    }

    std::optional<ComponentView> getComponentAt(sf::Vector2f position, float tolerance = 6.f) const {
        const sf::Vector2f halfSize{16.f, 7.f};
        for (const auto& [id, component] : components) {
            const sf::Vector2f delta{position.x - component.position.x, position.y - component.position.y};
            if (std::abs(delta.x) <= halfSize.x + tolerance && std::abs(delta.y) <= halfSize.y + tolerance) {
                return component;
            }
        }
        return std::nullopt;
    }

    static bool pointNearSegment(sf::Vector2f p, sf::Vector2f a, sf::Vector2f b, float tolerance) {
        const float minX = std::min(a.x, b.x) - tolerance;
        const float maxX = std::max(a.x, b.x) + tolerance;
        const float minY = std::min(a.y, b.y) - tolerance;
        const float maxY = std::max(a.y, b.y) + tolerance;

        if (p.x < minX || p.x > maxX || p.y < minY || p.y > maxY) {
            return false;
        }

        const sf::Vector2f ab = {b.x - a.x, b.y - a.y};
        const sf::Vector2f ap = {p.x - a.x, p.y - a.y};
        const float abLenSq = ab.x * ab.x + ab.y * ab.y;
        if (abLenSq < 1e-6f) {
            const float distSq = ap.x * ap.x + ap.y * ap.y;
            return distSq <= tolerance * tolerance;
        }
        const float t = std::max(0.f, std::min(1.f, (ap.x * ab.x + ap.y * ab.y) / abLenSq));
        const sf::Vector2f closest{a.x + ab.x * t, a.y + ab.y * t};
        const float dx = p.x - closest.x;
        const float dy = p.y - closest.y;
        return (dx * dx + dy * dy) <= tolerance * tolerance;
    }

    bool hasComponentAt(sf::Vector2f position, float tolerance = 4.f) const {
        const sf::Vector2f halfSize{16.f, 7.f};
        for (const auto& [id, component] : components) {
            const sf::Vector2f delta{position.x - component.position.x, position.y - component.position.y};
            if (std::abs(delta.x) <= halfSize.x + tolerance && std::abs(delta.y) <= halfSize.y + tolerance) {
                return true;
            }
        }
        return false;
    }

    std::optional<std::size_t> getWireIndexAt(sf::Vector2f position, float tolerance = 6.f) const {
        for (std::size_t i = 0; i < wires.size(); ++i) {
            const auto& wire = wires[i];
            auto startIt = nodePositions.find(wire.startNode);
            auto endIt = nodePositions.find(wire.endNode);
            if (startIt == nodePositions.end() || endIt == nodePositions.end()) {
                continue;
            }
            const sf::Vector2f start = startIt->second;
            const sf::Vector2f end = endIt->second;
            const sf::Vector2f delta = {end.x - start.x, end.y - start.y};
            const bool horizontalFirst = std::abs(delta.x) >= std::abs(delta.y);
            const sf::Vector2f elbow = horizontalFirst ? sf::Vector2f(end.x, start.y) : sf::Vector2f(start.x, end.y);

            if (pointNearSegment(position, start, elbow, tolerance)) {
                return i;
            }
            if (pointNearSegment(position, elbow, end, tolerance)) {
                return i;
            }
        }
        return std::nullopt;
    }

    std::optional<WireView> getWireAt(sf::Vector2f position, float tolerance = 6.f) const {
        if (auto index = getWireIndexAt(position, tolerance)) {
            return wires[*index];
        }
        return std::nullopt;
    }

    bool hasWireAt(sf::Vector2f position, float tolerance = 6.f) const {
        return getWireIndexAt(position, tolerance).has_value();
    }

    bool isNodeUsed(unsigned int nodeId) const {
        for (const auto& wire : wires) {
            if (wire.startNode == nodeId || wire.endNode == nodeId) {
                return true;
            }
        }
        for (const auto& [id, component] : components) {
            if (component.nodeA == nodeId || component.nodeB == nodeId) {
                return true;
            }
        }
        return false;
    }

    void removeNode(unsigned int nodeId) {
        nodePositions.erase(nodeId);
    }

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
