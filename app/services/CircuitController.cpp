//
// Created by Nazarii on 11/2/25.
//

#include "CircuitController.h"

#include <algorithm>
#include <cmath>
#include <type_traits>

#include <nlohmann/json.hpp>

inline sf::Vector2f toVector(const sf::Vector2f& value) {
    return value;
}

inline bool nearlyEqual(float a, float b, float eps = 0.5f) {
    return std::abs(a - b) <= eps;
}

inline bool nearlyEqual(const sf::Vector2f& a, const sf::Vector2f& b, float eps = 0.5f) {
    return nearlyEqual(a.x, b.x, eps) && nearlyEqual(a.y, b.y, eps);
}


CircuitController::CircuitController(CircuitService& service, CircuitView& view, const CoordinateTool& gridTool)
    : service(service), view(view), gridTool(gridTool) {
    cachedTopology = service.getCircuit().toJson().dump(2);
}

void CircuitController::handle(const CircuitCommand& command) {
    std::visit([
            this](auto&& cmd) {
                using T = std::decay_t<decltype(cmd)>;
                if constexpr (std::is_same_v<T, AddWireCommand>) {
                    handleWire(cmd);
                } else if constexpr (std::is_same_v<T, AddComponentCommand>) {
                    handleComponent(cmd);
                } else if constexpr (std::is_same_v<T, DeleteCommand>) {
                    handleDelete(cmd);
                }
            },
        command);

    cachedTopology = service.getCircuit().toJson().dump(2);
}

void CircuitController::handleWire(const AddWireCommand& command) {
    const sf::Vector2f snappedStart = gridTool.snapToGrid(toVector(command.start));
    const sf::Vector2f snappedEnd = gridTool.snapToGrid(toVector(command.end));

    if (snappedStart == snappedEnd) {
        return;
    }

    const unsigned int startNode = ensureNodeAt(snappedStart);
    const unsigned int endNode = ensureNodeAt(snappedEnd);

    const sf::Vector2f startPos = *view.getNodePosition(startNode);
    const sf::Vector2f endPos = *view.getNodePosition(endNode);

    if (startNode == endNode) {
        return;
    }

    unsigned int nodeA = ensureNodeAt(startPos);
    unsigned int nodeB = ensureNodeAt(endPos);
    unsigned int id = service.addComponent(ComponentType::Wire, nodeA, nodeB);
    view.recordComponent(id, ComponentType::Wire, sf::Vector2f(0,0), nodeA, nodeB);
}

void CircuitController::handleComponent(const AddComponentCommand& command) {
    const sf::Vector2f basePosition = gridTool.snapToGrid(toVector(command.position));
    const float spacing = static_cast<float>(gridTool.getSettings().spacing);

    const sf::Vector2f leftTerminal = gridTool.snapToGrid({basePosition.x - spacing, basePosition.y});
    const sf::Vector2f rightTerminal = gridTool.snapToGrid({basePosition.x + spacing, basePosition.y});

    const unsigned int nodeA = ensureNodeAt(leftTerminal);
    const unsigned int nodeB = ensureNodeAt(rightTerminal);

    const auto componentId = service.addComponent(command.type, nodeA, nodeB);
    view.recordComponent(componentId, command.type, basePosition, nodeA, nodeB);
}

void CircuitController::handleDelete(const DeleteCommand& command) {
    const sf::Vector2f snapped = gridTool.snapToGrid(toVector(command.position));
    if (auto component = view.removeComponentAt(snapped)) {
        service.removeComponent(component->type, component->nodeA, component->nodeB);
        cleanupNode(component->nodeA);
        cleanupNode(component->nodeB);
        return;
    }

    if (auto wire = view.removeWireAtPosition(snapped)) {
        service.removeComponent(ComponentType::Wire, wire->startNode, wire->endNode);
        cleanupNode(wire->startNode);
        cleanupNode(wire->endNode);
    }
}

unsigned int CircuitController::ensureNodeAt(sf::Vector2f position) {
    if (auto node = view.getNodeNear(position)) {
        return *node;
    }

    const sf::Vector2f snapped = gridTool.snapToGrid(position);
    const auto id = service.createNode();
    view.recordNode(id, snapped);
    return id;
}

void CircuitController::addWireSegments(const std::vector<std::pair<unsigned int, sf::Vector2f>>& orderedNodes) {
    if (orderedNodes.size() < 2) {
        return;
    }

    unsigned int prevNode = orderedNodes.front().first;
    for (std::size_t i = 1; i < orderedNodes.size(); ++i) {
        const unsigned int currentNode = orderedNodes[i].first;
        if (currentNode != prevNode) {
            const auto firstPos = view.getNodePosition(prevNode);
            const auto secondPos = view.getNodePosition(currentNode);
            if (firstPos && secondPos) {
                const sf::Vector2f mid = (*firstPos + *secondPos) * 0.5f;
                const auto wireId = service.addComponent(ComponentType::Wire, prevNode, currentNode);
                view.recordComponent(wireId, ComponentType::Wire, mid, prevNode, currentNode);
            }
        }
        prevNode = currentNode;
    }
}

void CircuitController::cleanupNode(unsigned int nodeId) {
    if (!view.isNodeUsed(nodeId)) {
        view.removeNode(nodeId);
        service.removeNode(nodeId);
    }
}
