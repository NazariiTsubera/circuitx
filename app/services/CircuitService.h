//
// Created by Nazarii on 11/1/25.
//

#ifndef CIRCUITSERVICE_H
#define CIRCUITSERVICE_H
#include <circuitx/circuit.hpp>
#include <variant>
#include <string>
#include "SFML/System/Vector2.hpp"


enum class ComponentType {
    Resistor = 0,
    Capacitor = 1,
    ISource = 2,
    VSource = 3
};

struct DeleteCommand { sf::Vector2f position; };
struct AddComponentCommand { sf::Vector2f position; ComponentType type; };
struct AddWireCommand { sf::Vector2f start; sf::Vector2f end; };

using CircuitCommand = std::variant<DeleteCommand, AddComponentCommand, AddWireCommand>;

/**
 * Owns and assembles circuit, manages it's lifecycle
 */
class CircuitService {
public:
    CircuitService();
    ~CircuitService();

    unsigned int createNode(const std::string& name = "");
    unsigned int addComponent(ComponentType type, unsigned int nodeA, unsigned int nodeB, float value = 1.0f);

    const circuitx::Circuit& getCircuit() const { return circuit; }
    circuitx::Circuit getCircuit() { return circuit; }

private:
    circuitx::Circuit circuit;
    unsigned int nextNodeId;
};


#endif //CIRCUITSERVICE_H
