//
// Created by Nazarii on 11/1/25.
//

#include "CircuitService.h"

#include <algorithm>
#include <sstream>

std::string makeNodeName(unsigned int id) {
    std::ostringstream oss;
    oss << "N" << id;
    return oss.str();
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
    if (nodeA == nodeB) {
        return 0;
    }

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
        case ComponentType::Wire:
            circuit.addElement(circuitx::Wire{nodeA, nodeB});
            break;
    }

    return nextComponentId++;
}

void CircuitService::removeComponent(ComponentType type, unsigned int nodeA, unsigned int nodeB) {
    auto& elements = circuit.elementsMutable();
    elements.erase(std::remove_if(elements.begin(), elements.end(), [&](const circuitx::Element& elem) {
                       switch (type) {
                           case ComponentType::Resistor:
                               if (const auto* res = std::get_if<circuitx::Res>(&elem)) {
                                   return (res->a == nodeA && res->b == nodeB) || (res->a == nodeB && res->b == nodeA);
                               }
                               break;
                           case ComponentType::Capacitor:
                               if (const auto* cap = std::get_if<circuitx::Cap>(&elem)) {
                                   return (cap->a == nodeA && cap->b == nodeB) || (cap->a == nodeB && cap->b == nodeA);
                               }
                               break;
                           case ComponentType::ISource:
                               if (const auto* isrc = std::get_if<circuitx::ISource>(&elem)) {
                                   return (isrc->a == nodeA && isrc->b == nodeB) || (isrc->a == nodeB && isrc->b == nodeA);
                               }
                               break;
                           case ComponentType::VSource:
                               if (const auto* vsrc = std::get_if<circuitx::VSource>(&elem)) {
                                   return (vsrc->a == nodeA && vsrc->b == nodeB) || (vsrc->a == nodeB && vsrc->b == nodeA);
                               }
                               break;
                           case ComponentType::Wire:
                               if (const auto* wire = std::get_if<circuitx::Wire>(&elem)) {
                                   return (wire->a == nodeA && wire->b == nodeB) || (wire->a == nodeB && wire->b == nodeA);
                               }
                               break;
                       }
                       return false;
                   }),
        elements.end());
}

void CircuitService::removeNode(unsigned int nodeId) {
    auto& elements = circuit.elementsMutable();
    elements.erase(std::remove_if(elements.begin(), elements.end(), [&](const circuitx::Element& elem) {
                       if (const auto* wire = std::get_if<circuitx::Wire>(&elem)) {
                           return wire->a == nodeId || wire->b == nodeId;
                       }
                       return false;
                   }),
        elements.end());

    auto& nodes = circuit.nodesMutable();
    nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [&](const circuitx::Node& node) { return node.id == nodeId; }), nodes.end());
}

std::optional<float> CircuitService::getComponentValue(ComponentType type, unsigned int nodeA, unsigned int nodeB) const {
    auto matches = [&](unsigned int first, unsigned int second) {
        return (first == nodeA && second == nodeB) || (first == nodeB && second == nodeA);
    };

    const auto elementsSnapshot = circuit.getElements();
    for (const auto& elem : elementsSnapshot) {
        switch (type) {
            case ComponentType::Resistor:
                if (const auto* res = std::get_if<circuitx::Res>(&elem)) {
                    if (matches(res->a, res->b)) {
                        return res->res;
                    }
                }
                break;
            case ComponentType::Capacitor:
                if (const auto* cap = std::get_if<circuitx::Cap>(&elem)) {
                    if (matches(cap->a, cap->b)) {
                        return cap->cap;
                    }
                }
                break;
            case ComponentType::ISource:
                if (const auto* isrc = std::get_if<circuitx::ISource>(&elem)) {
                    if (matches(isrc->a, isrc->b)) {
                        return isrc->cur;
                    }
                }
                break;
            case ComponentType::VSource:
                if (const auto* vsrc = std::get_if<circuitx::VSource>(&elem)) {
                    if (matches(vsrc->a, vsrc->b)) {
                        return vsrc->vol;
                    }
                }
                break;
            case ComponentType::Wire:
                break;
        }
    }

    return std::nullopt;
}

bool CircuitService::updateComponentValue(ComponentType type, unsigned int nodeA, unsigned int nodeB, float value) {
    auto matches = [&](unsigned int first, unsigned int second) {
        return (first == nodeA && second == nodeB) || (first == nodeB && second == nodeA);
    };

    auto& elements = circuit.elementsMutable();
    for (auto& elem : elements) {
        switch (type) {
            case ComponentType::Resistor:
                if (auto* res = std::get_if<circuitx::Res>(&elem)) {
                    if (matches(res->a, res->b)) {
                        res->res = value;
                        return true;
                    }
                }
                break;
            case ComponentType::Capacitor:
                if (auto* cap = std::get_if<circuitx::Cap>(&elem)) {
                    if (matches(cap->a, cap->b)) {
                        cap->cap = value;
                        return true;
                    }
                }
                break;
            case ComponentType::ISource:
                if (auto* isrc = std::get_if<circuitx::ISource>(&elem)) {
                    if (matches(isrc->a, isrc->b)) {
                        isrc->cur = value;
                        return true;
                    }
                }
                break;
            case ComponentType::VSource:
                if (auto* vsrc = std::get_if<circuitx::VSource>(&elem)) {
                    if (matches(vsrc->a, vsrc->b)) {
                        vsrc->vol = value;
                        return true;
                    }
                }
                break;
            case ComponentType::Wire:
                break;
        }
    }

    return false;
}
