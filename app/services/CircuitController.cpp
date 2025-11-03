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

CircuitController::CircuitController(CircuitService& service, CircuitView& view)
    : service(service), view(view) {
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
    auto startPos = toVector(command.start);
    auto endPos = toVector(command.end);

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
    auto position = toVector(command.position);

    const auto nodeA = service.createNode();
    const auto nodeB = service.createNode();
    view.recordNode(nodeA, position);
    view.recordNode(nodeB, position);

    service.addComponent(command.type, nodeA, nodeB);
}
