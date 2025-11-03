//
// Created by Nazarii on 11/1/25.
//

#include "CircuitService.h"

#include <sstream>

namespace {
std::string makeNodeName(unsigned int id) {
    std::ostringstream oss;
    oss << "N" << id;
    return oss.str();
}
}

CircuitService::CircuitService()
    : circuit(), nextNodeId(1), nextComponentId(1) {}

CircuitService::~CircuitService() = default;

unsigned int CircuitService::createNode(const std::string& name) {
    const unsigned int id = nextNodeId++;
    circuit.addNode({id, name.empty() ? makeNodeName(id) : name});
    return id;
}

unsigned int CircuitService::addComponent(ComponentType type, unsigned int nodeA, unsigned int nodeB, float value) {
    switch (type) {
        case ComponentType::Resistor:
            circuit.addElement(circuitx::Res{nodeA, nodeB, value});
            break;
        case ComponentType::Capacitor:
            circuit.addElement(circuitx::Cap{nodeA, nodeB, value});
            break;
        case ComponentType::ISource:
            circuit.addElement(circuitx::ISource{nodeA, nodeB, value});
            break;
        case ComponentType::VSource:
            circuit.addElement(circuitx::VSource{nodeA, nodeB, value});
            break;
    }

    return nextComponentId++;
}
