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

struct Segment {
    sf::Vector2f a;
    sf::Vector2f b;

    bool isHorizontal() const { return std::abs(a.y - b.y) < 1e-4f; }
    bool isVertical() const { return std::abs(a.x - b.x) < 1e-4f; }
};

std::optional<sf::Vector2f> intersectSegments(const Segment& first, const Segment& second) {
    if (first.isHorizontal() && second.isVertical()) {
        const float hx1 = std::min(first.a.x, first.b.x);
        const float hx2 = std::max(first.a.x, first.b.x);
        const float vx = second.a.x;
        const float hy = first.a.y;
        const float vy1 = std::min(second.a.y, second.b.y);
        const float vy2 = std::max(second.a.y, second.b.y);

        if (vx >= hx1 && vx <= hx2 && hy >= vy1 && hy <= vy2) {
            return sf::Vector2f{vx, hy};
        }
        return std::nullopt;
    }

    if (first.isVertical() && second.isHorizontal()) {
        return intersectSegments(second, first);
    }

    return std::nullopt;
}

float computePathParameter(sf::Vector2f point, sf::Vector2f start, sf::Vector2f elbow, sf::Vector2f end, bool horizontalFirst) {
    if (horizontalFirst) {
        const float firstLength = std::abs(elbow.x - start.x);
        if (nearlyEqual(point.y, start.y)) {
            return std::abs(point.x - start.x);
        }
        return firstLength + std::abs(point.y - elbow.y);
    }

    const float firstLength = std::abs(elbow.y - start.y);
    if (nearlyEqual(point.x, start.x)) {
        return std::abs(point.y - start.y);
    }
    return firstLength + std::abs(point.x - elbow.x);
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

    const unsigned int startNode = ensureNodeAt(snappedStart);
    const unsigned int endNode = ensureNodeAt(snappedEnd);

    const sf::Vector2f startPos = *view.getNodePosition(startNode);
    const sf::Vector2f endPos = *view.getNodePosition(endNode);

    if (startNode == endNode) {
        return;
    }

    const bool horizontalFirst = std::abs(endPos.x - startPos.x) >= std::abs(endPos.y - startPos.y);
    const sf::Vector2f elbow = horizontalFirst ? sf::Vector2f(endPos.x, startPos.y) : sf::Vector2f(startPos.x, endPos.y);
    const Segment newSegments[2] = {{startPos, elbow}, {elbow, endPos}};

    struct PathNode {
        float param;
        unsigned int nodeId;
        sf::Vector2f position;
    };

    struct WireSplit {
        std::size_t index;
        unsigned int startNode;
        unsigned int endNode;
        unsigned int junctionNode;
    };

    std::vector<PathNode> pathNodes;
    const float totalLength = horizontalFirst
        ? std::abs(elbow.x - startPos.x) + std::abs(endPos.y - elbow.y)
        : std::abs(elbow.y - startPos.y) + std::abs(endPos.x - elbow.x);
    pathNodes.push_back({0.f, startNode, startPos});
    pathNodes.push_back({totalLength, endNode, endPos});

    std::vector<WireSplit> splits;

    for (std::size_t i = 0; i < view.wireCount(); ++i) {
        const WireView& existing = view.getWire(i);
        const auto startExistingOpt = view.getNodePosition(existing.startNode);
        const auto endExistingOpt = view.getNodePosition(existing.endNode);
        if (!startExistingOpt || !endExistingOpt) {
            continue;
        }

        const sf::Vector2f existingStart = *startExistingOpt;
        const sf::Vector2f existingEnd = *endExistingOpt;
        const bool existingHorizontalFirst = std::abs(existingEnd.x - existingStart.x) >= std::abs(existingEnd.y - existingStart.y);
        const sf::Vector2f existingElbow = existingHorizontalFirst ? sf::Vector2f(existingEnd.x, existingStart.y) : sf::Vector2f(existingStart.x, existingEnd.y);
        const Segment existingSegments[2] = {{existingStart, existingElbow}, {existingElbow, existingEnd}};

        if (existing.startNode == startNode || existing.startNode == endNode ||
            existing.endNode == startNode || existing.endNode == endNode) {
            continue;
        }

        bool intersected = false;
        for (const Segment& newSeg : newSegments) {
            for (const Segment& existingSeg : existingSegments) {
                const auto intersection = intersectSegments(newSeg, existingSeg);
                if (!intersection) {
                    continue;
                }

                const sf::Vector2f intersectionPos = gridTool.snapToGrid(*intersection);
                if (nearlyEqual(intersectionPos, startPos) || nearlyEqual(intersectionPos, endPos) ||
                    nearlyEqual(intersectionPos, existingStart) || nearlyEqual(intersectionPos, existingEnd)) {
                    continue;
                }

                const unsigned int junctionNode = ensureNodeAt(intersectionPos);
                pathNodes.push_back({computePathParameter(intersectionPos, startPos, elbow, endPos, horizontalFirst), junctionNode, intersectionPos});
                splits.push_back({i, existing.startNode, existing.endNode, junctionNode});
                intersected = true;
                break;
            }
            if (intersected) {
                break;
            }
        }
    }

    std::sort(pathNodes.begin(), pathNodes.end(), [](const PathNode& lhs, const PathNode& rhs) {
        if (std::abs(lhs.param - rhs.param) < 1e-3f) {
            return lhs.nodeId < rhs.nodeId;
        }
        return lhs.param < rhs.param;
    });

    pathNodes.erase(std::unique(pathNodes.begin(), pathNodes.end(), [](const PathNode& lhs, const PathNode& rhs) {
                        return (std::abs(lhs.param - rhs.param) < 1e-3f) || lhs.nodeId == rhs.nodeId;
                    }),
        pathNodes.end());

    std::sort(splits.begin(), splits.end(), [](const WireSplit& lhs, const WireSplit& rhs) {
        return lhs.index > rhs.index;
    });

    for (const WireSplit& split : splits) {
        service.removeComponent(ComponentType::Wire, split.startNode, split.endNode);
        view.removeWireAt(split.index);
        const sf::Vector2f firstMid = (*view.getNodePosition(split.startNode) + *view.getNodePosition(split.junctionNode)) * 0.5f;
        const sf::Vector2f secondMid = (*view.getNodePosition(split.junctionNode) + *view.getNodePosition(split.endNode)) * 0.5f;
        const auto firstId = service.addComponent(ComponentType::Wire, split.startNode, split.junctionNode);
        view.recordComponent(firstId, ComponentType::Wire, firstMid, split.startNode, split.junctionNode);
        const auto secondId = service.addComponent(ComponentType::Wire, split.junctionNode, split.endNode);
        view.recordComponent(secondId, ComponentType::Wire, secondMid, split.junctionNode, split.endNode);
    }

    std::vector<std::pair<unsigned int, sf::Vector2f>> ordered;
    ordered.reserve(pathNodes.size());
    for (const auto& node : pathNodes) {
        ordered.emplace_back(node.nodeId, node.position);
    }

    addWireSegments(ordered);
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
