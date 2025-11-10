//
// Created by Nazarii on 11/20/25.
//

#include "CircuitSimulator.h"

#include <Eigen/Core>
#include <functional>
#include <map>
#include <sstream>
#include <unordered_map>
#include <variant>

#include "../services/ComponentType.h"

namespace {
std::string formatValue(double value) {
    std::ostringstream ss;
    ss.setf(std::ios::fixed, std::ios::floatfield);
    ss.precision(6);
    ss << value;
    return ss.str();
}

std::string elementHeader(const std::function<std::string(unsigned int)>& nameForNode,
                          const std::string& label,
                          unsigned int nodeA,
                          unsigned int nodeB) {
    std::ostringstream header;
    header << "  " << label << " (" << nameForNode(nodeA) << " -> " << nameForNode(nodeB) << ")";
    return header.str();
}
} // namespace

CircuitSimulator::CircuitSimulator() {
    simulationResult.headline = "Awaiting simulation run.";
    simulationResult.textualReport = "Press Play to run a DC analysis on the current circuit.";
}

void CircuitSimulator::runDcAnalysis(circuitx::Circuit circuit) {
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
            const double voltageDrop = voltageAt(res->a) - voltageAt(res->b);
            double current = 0.0;
            if (res->res > 0.0f) {
                current = voltageDrop / static_cast<double>(res->res);
            }
            oss << elementHeader(nameForNode, label, res->a, res->b) << "\n";
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
            const double voltageDrop = voltageAt(cap->a) - voltageAt(cap->b);
            oss << elementHeader(nameForNode, label, cap->a, cap->b) << "\n";
            oss << "    ΔV = " << voltageDrop << " V, C = " << cap->cap << " F (transient current not computed)\n";
            elementData.type = ComponentType::Capacitor;
            elementData.label = label;
            elementData.nodeA = cap->a;
            elementData.nodeB = cap->b;
            elementData.voltageDrop = voltageDrop;
            elementData.detail = "C = " + formatValue(cap->cap) + " F (transient current not computed)";
        } else if (const auto* isrc = std::get_if<circuitx::ISource>(&elem)) {
            const std::string label = nextLabel(ComponentType::ISource);
            const double voltageDrop = voltageAt(isrc->a) - voltageAt(isrc->b);
            oss << elementHeader(nameForNode, label, isrc->a, isrc->b) << "\n";
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
            oss << elementHeader(nameForNode, label, vsrc->a, vsrc->b) << "\n";
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

void CircuitSimulator::runTransient(CircuitService& service, double durationSeconds, double timestepSeconds) {
    transientSimulation = service.simulateTransient(durationSeconds, timestepSeconds);
}
