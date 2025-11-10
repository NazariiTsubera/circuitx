//
// Created by Nazarii on 11/20/25.
//

#include "CircuitEditor.h"

#include <algorithm>
#include <map>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

namespace {
sf::Vector2f snapVector(const CoordinateTool& tool, const sf::Vector2f& value) {
    return tool.snapToGrid(value);
}

std::pair<sf::Vector2f, sf::Vector2f> computeTerminals(const CoordinateTool& gridTool,
                                                       const sf::Vector2f& center,
                                                       float spacing,
                                                       int rotationSteps) {
    const int rotation = normalizeRotationSteps(rotationSteps);
    sf::Vector2f offsetA{};
    sf::Vector2f offsetB{};
    switch (rotation) {
        case 0:
            offsetA = {-spacing, 0.f};
            offsetB = {spacing, 0.f};
            break;
        case 1:
            offsetA = {0.f, spacing};
            offsetB = {0.f, -spacing};
            break;
        case 2:
            offsetA = {spacing, 0.f};
            offsetB = {-spacing, 0.f};
            break;
        case 3:
            offsetA = {0.f, -spacing};
            offsetB = {0.f, spacing};
            break;
        default:
            break;
    }

    const sf::Vector2f terminalA = gridTool.snapToGrid(center + offsetA);
    const sf::Vector2f terminalB = gridTool.snapToGrid(center + offsetB);
    return {terminalA, terminalB};
}
} // namespace

CircuitEditor::CircuitEditor(CircuitService& service, CircuitView& view, const CoordinateTool& gridTool)
    : service(service),
      view(view),
      gridTool(gridTool) {
    refreshTopology();
}

void CircuitEditor::execute(const CircuitCommand& command) {
    std::visit(
        [this](auto&& cmd) {
            using T = std::decay_t<decltype(cmd)>;
            if constexpr (std::is_same_v<T, AddWireCommand>) {
                addWire(cmd);
            } else if constexpr (std::is_same_v<T, AddComponentCommand>) {
                addComponent(cmd);
            } else if constexpr (std::is_same_v<T, DeleteCommand>) {
                deleteAt(cmd);
            }
        },
        command);

    refreshTopology();
}

void CircuitEditor::addWire(const AddWireCommand& command) {
    const sf::Vector2f snappedStart = snapVector(gridTool, command.start);
    const sf::Vector2f snappedEnd = snapVector(gridTool, command.end);

    if (snappedStart == snappedEnd) {
        return;
    }

    const unsigned int startNode = ensureNodeAt(snappedStart);
    const unsigned int endNode = ensureNodeAt(snappedEnd);
    if (startNode == endNode) {
        return;
    }

    const auto wireId = service.addComponent(ComponentType::Wire, startNode, endNode);
    const auto midpoint = (snappedStart + snappedEnd) * 0.5f;
    view.recordComponent(wireId, ComponentType::Wire, midpoint, startNode, endNode);
}

void CircuitEditor::addComponent(const AddComponentCommand& command) {
    const sf::Vector2f basePosition = snapVector(gridTool, command.position);
    const float spacing = static_cast<float>(gridTool.getSettings().spacing);
    const auto [terminalA, terminalB] = computeTerminals(gridTool, basePosition, spacing, command.rotationSteps);

    const unsigned int nodeA = ensureNodeAt(terminalA);
    const unsigned int nodeB = ensureNodeAt(terminalB);

    const auto componentId = service.addComponent(command.type, nodeA, nodeB);
    view.recordComponent(componentId,
        command.type,
        basePosition,
        nodeA,
        nodeB,
        normalizeRotationSteps(command.rotationSteps));
}

void CircuitEditor::deleteAt(const DeleteCommand& command) {
    const sf::Vector2f raw = command.position;

    auto tryErase = [&](const sf::Vector2f& target) -> bool {
        if (auto component = view.removeComponentAt(target)) {
            service.removeComponent(component->type, component->nodeA, component->nodeB);
            cleanupNode(component->nodeA);
            cleanupNode(component->nodeB);
            return true;
        }

        if (auto wire = view.removeWireAtPosition(target)) {
            service.removeComponent(ComponentType::Wire, wire->startNode, wire->endNode);
            cleanupNode(wire->startNode);
            cleanupNode(wire->endNode);
            return true;
        }
        return false;
    };

    if (tryErase(raw)) {
        return;
    }

    const sf::Vector2f snapped = snapVector(gridTool, raw);
    if ((snapped.x != raw.x) || (snapped.y != raw.y)) {
        tryErase(snapped);
    }
}

void CircuitEditor::deleteWire(const WireView& wire) {
    if (view.removeWire(wire.startNode, wire.endNode)) {
        service.removeComponent(ComponentType::Wire, wire.startNode, wire.endNode);
        cleanupNode(wire.startNode);
        cleanupNode(wire.endNode);
        refreshTopology();
    }
}

bool CircuitEditor::rotateComponent(unsigned int componentId, int rotationDelta) {
    auto componentOpt = view.getComponent(componentId);
    if (!componentOpt || componentOpt->type == ComponentType::Wire) {
        return false;
    }

    ComponentView component = *componentOpt;
    const int newRotation = normalizeRotationSteps(component.rotationSteps + rotationDelta);
    const float spacing = static_cast<float>(gridTool.getSettings().spacing);
    const auto [terminalA, terminalB] = computeTerminals(gridTool, component.position, spacing, newRotation);

    view.recordNode(component.nodeA, terminalA);
    view.recordNode(component.nodeB, terminalB);
    view.setComponentRotation(componentId, newRotation);
    refreshTopology();
    return true;
}

std::optional<ComponentView> CircuitEditor::getComponentAt(sf::Vector2f position, float tolerance) const {
    return view.getComponentAt(position, tolerance);
}

std::optional<WireView> CircuitEditor::getWireAt(sf::Vector2f position, float tolerance) const {
    return view.getWireAt(position, tolerance);
}

std::optional<ComponentView> CircuitEditor::getComponent(unsigned int componentId) const {
    return view.getComponent(componentId);
}

std::optional<float> CircuitEditor::getComponentValue(const ComponentView& component) const {
    return service.getComponentValue(component.type, component.nodeA, component.nodeB);
}

bool CircuitEditor::updateComponentValue(const ComponentView& component, float newValue) {
    if (service.updateComponentValue(component.type, component.nodeA, component.nodeB, newValue)) {
        refreshTopology();
        return true;
    }
    return false;
}

bool CircuitEditor::hasSelectableAt(sf::Vector2f position) const {
    return view.hasSelectableAt(position);
}

std::unordered_map<unsigned int, std::string> CircuitEditor::buildComponentLabels() const {
    std::vector<ComponentView> components;
    components.reserve(view.getComponents().size());
    for (const auto& [componentId, component] : view.getComponents()) {
        if (component.type == ComponentType::Wire) {
            continue;
        }
        components.push_back(component);
    }

    std::sort(components.begin(), components.end(), [](const ComponentView& lhs, const ComponentView& rhs) {
        return lhs.id < rhs.id;
    });

    std::map<ComponentType, std::size_t> counters;
    std::unordered_map<unsigned int, std::string> labels;
    labels.reserve(components.size());

    for (const auto& component : components) {
        std::size_t index = ++counters[component.type];
        auto label = componentLabel(component.type, index);
        if (!label.empty()) {
            labels.emplace(component.id, std::move(label));
        }
    }

    return labels;
}

void CircuitEditor::refreshTopology() {
    cachedTopology = service.getCircuit().toJson().dump(2);
}

unsigned int CircuitEditor::ensureNodeAt(sf::Vector2f position) {
    if (auto node = view.getNodeNear(position)) {
        return *node;
    }

    const sf::Vector2f snapped = snapVector(gridTool, position);
    const auto id = service.createNode();
    view.recordNode(id, snapped);
    return id;
}

void CircuitEditor::cleanupNode(unsigned int nodeId) {
    if (!view.isNodeUsed(nodeId)) {
        view.removeNode(nodeId);
        service.removeNode(nodeId);
    }
}
