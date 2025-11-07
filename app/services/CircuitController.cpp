//
// Created by Nazarii on 11/2/25.
//

#include "CircuitController.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <unordered_map>
#include <map>
#include <sstream>
#include <Eigen/Core>

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

inline std::pair<sf::Vector2f, sf::Vector2f> computeTerminals(const CoordinateTool& gridTool,
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


CircuitController::CircuitController(CircuitService& service, CircuitView& view, const CoordinateTool& gridTool)
    : service(service), view(view), gridTool(gridTool) {
    cachedTopology = service.getCircuit().toJson().dump(2);
    simulationResult.headline = "Awaiting simulation run.";
    simulationResult.textualReport = "Press Play to run a DC analysis on the current circuit.";
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

    refreshTopologyCache();
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

    const auto [terminalA, terminalB] = computeTerminals(gridTool, basePosition, spacing, command.rotationSteps);

    const unsigned int nodeA = ensureNodeAt(terminalA);
    const unsigned int nodeB = ensureNodeAt(terminalB);

    const auto componentId = service.addComponent(command.type, nodeA, nodeB);
    view.recordComponent(componentId, command.type, basePosition, nodeA, nodeB, normalizeRotationSteps(command.rotationSteps));
}

void CircuitController::handleDelete(const DeleteCommand& command) {
    const sf::Vector2f raw = toVector(command.position);

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

    const sf::Vector2f snapped = gridTool.snapToGrid(raw);
    if ((snapped.x != raw.x) || (snapped.y != raw.y)) {
        tryErase(snapped);
    }
}

bool CircuitController::rotateComponent(unsigned int componentId, int rotationDelta) {
    auto componentOpt = view.getComponent(componentId);
    if (!componentOpt.has_value()) {
        return false;
    }
    ComponentView component = *componentOpt;
    if (component.type == ComponentType::Wire) {
        return false;
    }

    const int newRotation = normalizeRotationSteps(component.rotationSteps + rotationDelta);
    const float spacing = static_cast<float>(gridTool.getSettings().spacing);
    const auto [terminalA, terminalB] = computeTerminals(gridTool, component.position, spacing, newRotation);

    view.recordNode(component.nodeA, terminalA);
    view.recordNode(component.nodeB, terminalB);
    view.setComponentRotation(componentId, newRotation);
    return true;
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


//Bad design, i will redo later
void CircuitController::simulate() {
    circuitx::Circuit circuit = service.getCircuit();
    simulationResult.clear();

    const Eigen::VectorXd solution = circuit.solve();
    const auto& nodeOrder = circuit.solutionNodeOrdering();
    const auto& voltageOrder = circuit.solutionVoltageOrdering();
    const auto groundId = circuit.solutionGround();

    if (solution.size() == 0) {
        simulationResult.headline = "Empty circuit.";
        simulationResult.textualReport = "System has no unknowns (empty circuit).";
        return;
    }

    std::unordered_map<unsigned int, std::string> nameById;
    for (const auto& node : circuit.getNodes()) {
        nameById[node.id] = node.name;
    }

    auto nameForNode = [&](unsigned int id) {
        if (id == groundId) {
            return std::string("GND");
        }
        if (auto it = nameById.find(id); it != nameById.end()) {
            return it->second;
        }
        return std::string("N") + std::to_string(id);
    };

    std::unordered_map<unsigned int, double> nodeVoltages;
    nodeVoltages[groundId] = 0.0;
    for (std::size_t i = 0; i < nodeOrder.size(); ++i) {
        nodeVoltages[nodeOrder[i]] = solution(static_cast<Eigen::Index>(i));
    }

    auto voltageAt = [&](unsigned int nodeId) {
        if (auto it = nodeVoltages.find(nodeId); it != nodeVoltages.end()) {
            return it->second;
        }
        return 0.0;
    };

    std::ostringstream oss;
    oss.setf(std::ios::fixed, std::ios::floatfield);
    oss.precision(6);
    auto formatValue = [] (double value) {
        std::ostringstream ss;
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss.precision(6);
        ss << value;
        return ss.str();
    };

    oss << "Reference node: " << nameForNode(groundId) << " (ID " << groundId << ")\n\n";

    oss << "Node Voltages:\n";
    if (nodeOrder.empty()) {
        oss << "  (only reference node present)\n";
    } else {
        for (unsigned int nodeId : nodeOrder) {
            oss << "  V(" << nameForNode(nodeId) << ") = " << nodeVoltages[nodeId] << " V\n";
        }
    }

    const auto elements = circuit.getElements();
    if (!elements.empty()) {
        oss << "\nElement Quantities:\n";
    }

    std::map<ComponentType, std::size_t> elementCounters;

    auto nextLabel = [&](ComponentType type) {
        std::size_t index = ++elementCounters[type];
        return componentLabel(type, index);
    };

    auto elementHeader = [&](const std::string& label, unsigned int a, unsigned int b) {
        std::ostringstream header;
        header << "  " << label << " (" << nameForNode(a) << " -> " << nameForNode(b) << ")";
        return header.str();
    };

    std::unordered_map<std::size_t, std::size_t> voltageIndexMap;
    for (std::size_t idx = 0; idx < voltageOrder.size(); ++idx) {
        voltageIndexMap[voltageOrder[idx]] = idx;
    }

    simulationResult.solved = true;
    simulationResult.referenceNodeId = groundId;
    simulationResult.referenceNodeName = nameForNode(groundId);
    simulationResult.headline = "DC operating point solved.";
    simulationResult.nodes.clear();
    simulationResult.nodes.reserve(nodeOrder.size());
    for (unsigned int nodeId : nodeOrder) {
        simulationResult.nodes.push_back({nodeId, nameForNode(nodeId), nodeVoltages[nodeId]});
    }
    simulationResult.elements.clear();
    simulationResult.elements.reserve(elements.size());

    for (std::size_t elemIdx = 0; elemIdx < elements.size(); ++elemIdx) {
        const auto& elem = elements[elemIdx];
        SimulationElementResult elementData;
        if (const auto* res = std::get_if<circuitx::Res>(&elem)) {
            const std::string label = nextLabel(ComponentType::Resistor);
            const double va = voltageAt(res->a);
            const double vb = voltageAt(res->b);
            const double voltageDrop = va - vb;
            double current = 0.0;
            if (res->res > 0.0f) {
                current = voltageDrop / static_cast<double>(res->res);
            }
            oss << elementHeader(label, res->a, res->b) << "\n";
            oss << "    ΔV = " << voltageDrop << " V, I = " << current << " A, R = " << res->res << " Ω\n";
            elementData.type = ComponentType::Resistor;
            elementData.label = label;
            elementData.nodeA = res->a;
            elementData.nodeB = res->b;
            elementData.voltageDrop = voltageDrop;
            if (res->res > 0.0f) {
                elementData.current = current;
            }
            elementData.detail = "R = " + formatValue(res->res) + " Ω";
        } else if (const auto* cap = std::get_if<circuitx::Cap>(&elem)) {
            const std::string label = nextLabel(ComponentType::Capacitor);
            const double va = voltageAt(cap->a);
            const double vb = voltageAt(cap->b);
            const double voltageDrop = va - vb;
            oss << elementHeader(label, cap->a, cap->b) << "\n";
            oss << "    ΔV = " << voltageDrop << " V, C = " << cap->cap << " F (transient current not computed)\n";
            elementData.type = ComponentType::Capacitor;
            elementData.label = label;
            elementData.nodeA = cap->a;
            elementData.nodeB = cap->b;
            elementData.voltageDrop = voltageDrop;
            elementData.detail = "C = " + formatValue(cap->cap) + " F (transient current not computed)";
        } else if (const auto* isrc = std::get_if<circuitx::ISource>(&elem)) {
            const std::string label = nextLabel(ComponentType::ISource);
            const double va = voltageAt(isrc->a);
            const double vb = voltageAt(isrc->b);
            const double voltageDrop = va - vb;
            oss << elementHeader(label, isrc->a, isrc->b) << "\n";
            oss << "    ΔV = " << voltageDrop << " V, I = " << isrc->cur << " A (positive from A to B)\n";
            elementData.type = ComponentType::ISource;
            elementData.label = label;
            elementData.nodeA = isrc->a;
            elementData.nodeB = isrc->b;
            elementData.voltageDrop = voltageDrop;
            elementData.current = isrc->cur;
            elementData.detail = "Iset = " + formatValue(isrc->cur) + " A (positive from A to B)";
        } else if (const auto* vsrc = std::get_if<circuitx::VSource>(&elem)) {
            const std::string label = nextLabel(ComponentType::VSource);
            const double va = voltageAt(vsrc->a);
            const double vb = voltageAt(vsrc->b);
            double equationCurrent = 0.0;
            if (auto it = voltageIndexMap.find(elemIdx); it != voltageIndexMap.end()) {
                equationCurrent = solution(static_cast<Eigen::Index>(nodeOrder.size() + it->second));
            }
            oss << elementHeader(label, vsrc->a, vsrc->b) << "\n";
            oss << "    Va = " << va << " V, Vb = " << vb << " V, Source = " << vsrc->vol
                << " V, I = " << equationCurrent << " A (positive from A to B)\n";
            elementData.type = ComponentType::VSource;
            elementData.label = label;
            elementData.nodeA = vsrc->a;
            elementData.nodeB = vsrc->b;
            elementData.voltageDrop = va - vb;
            elementData.current = equationCurrent;
            elementData.detail = "Vs = " + formatValue(vsrc->vol) + " V";
        }
        if (!elementData.label.empty()) {
            simulationResult.elements.push_back(std::move(elementData));
        }
    }

    simulationResult.textualReport = oss.str();
}

std::optional<ComponentView> CircuitController::getComponentAt(sf::Vector2f position, float tolerance) const {
    return view.getComponentAt(position, tolerance);
}

std::optional<WireView> CircuitController::getWireAt(sf::Vector2f position, float tolerance) const {
    return view.getWireAt(position, tolerance);
}

std::optional<float> CircuitController::getComponentValue(const ComponentView& component) const {
    return service.getComponentValue(component.type, component.nodeA, component.nodeB);
}

bool CircuitController::updateComponentValue(const ComponentView& component, float newValue) {
    if (service.updateComponentValue(component.type, component.nodeA, component.nodeB, newValue)) {
        refreshTopologyCache();
        return true;
    }
    return false;
}

void CircuitController::refreshTopologyCache() {
    cachedTopology = service.getCircuit().toJson().dump(2);
}

std::unordered_map<unsigned int, std::string> CircuitController::buildComponentLabels() const {
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
