//
// Created by Nazarii on 11/1/25.
//

#ifndef CIRCUITSERVICE_H
#define CIRCUITSERVICE_H
#include <circuitx/circuit.hpp>
#include "SFML/System/Vector2.hpp"



enum class ComponentType {
    Resistor = 0,
    Capacitor = 1,
    ISource = 2,
    VSource = 3
};

struct DeleteCommand {sf::Vector2f position;};
struct AddComponentCommand {sf::Vector2f position; ComponentType type; };
struct AddWireCommand {sf::Vector2f start; sf::Vector2f end;};

struct NodeChange {
    unsigned int id;
    sf::Vector2f position;
    bool created;
};


struct WireResult {
    NodeChange origin;
    NodeChange end;
};

struct ErrorResult {
    std::string error;
};


typedef std::variant<DeleteCommand, AddComponentCommand, AddWireCommand> CircuitCommand;
typedef std::variant<ErrorResult, WireResult> CircuitResult;

/**
 * Owns and assembles circuit, manages it's lifecycle
 */

class CircuitService {
private:
    unsigned int nextId;
public:
    CircuitService();
    virtual ~CircuitService();

    CircuitResult submit(CircuitCommand cmd);



    circuitx::Circuit getCircuit() { return circuit; }
private:
    circuitx::Circuit circuit;
};



#endif //CIRCUITSERVICE_H
