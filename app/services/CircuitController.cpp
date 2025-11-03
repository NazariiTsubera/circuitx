//
// Created by Nazarii on 11/2/25.
//

#include "CircuitController.h"

#include <algorithm>
#include <type_traits>

#include <nlohmann/json.hpp>

namespace {
inline sf::Vector2f toVector(const sf::Vector2f& value) {
    return value;
}
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
                } else {
                }
            },
        command);

    cachedTopology = service.getCircuit().toJson().dump(2);
}

void CircuitController::handleWire(const AddWireCommand& command) {
    auto startPos = gridTool.snapToGrid(toVector(command.start));
    auto endPos = gridTool.snapToGrid(toVector(command.end));

    auto startNode = view.getNodeNear(startPos);
    if (!startNode) {
        const auto id = service.createNode();
        view.recordNode(id, startPos);
        startNode = id;
    }

    auto endNode = view.getNodeNear(endPos);
    if (!endNode) {
        const auto id = service.createNode();
        view.recordNode(id, endPos);
        endNode = id;
    }

    if (*startNode != *endNode) {
        view.recordWire(*startNode, *endNode);
    }
}

void CircuitController::handleComponent(const AddComponentCommand& command) {
    auto basePosition = gridTool.snapToGrid(toVector(command.position));
    const float spacing = static_cast<float>(gridTool.getSettings().spacing);

    const sf::Vector2f leftTerminal = gridTool.snapToGrid({basePosition.x - spacing, basePosition.y});
    const sf::Vector2f rightTerminal = gridTool.snapToGrid({basePosition.x + spacing, basePosition.y});

    auto leftNodeId = view.getNodeNear(leftTerminal);
    unsigned int nodeA = leftNodeId ? *leftNodeId : service.createNode();
    if (!leftNodeId) {
        view.recordNode(nodeA, leftTerminal);
    }

    auto rightNodeId = view.getNodeNear(rightTerminal);
    unsigned int nodeB = rightNodeId ? *rightNodeId : service.createNode();
    if (!rightNodeId) {
        view.recordNode(nodeB, rightTerminal);
    }

    const auto componentId = service.addComponent(command.type, nodeA, nodeB);
    view.recordComponent(componentId, command.type, basePosition);
}
