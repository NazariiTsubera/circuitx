//
// Created by Nazarii on 11/1/25.
//

#ifndef CIRCUITSERVICE_H
#define CIRCUITSERVICE_H
#include <circuitx/circuit.hpp>

#include "SFML/System/Vector2.hpp"

enum class CommandType {
    DeleteComp,
    AddComp,
    AddWire
};

struct DeleteCommand {sf::Vector2f position;};
struct AddComponentCommand {sf::Vector2f position; CommandType type; };
struct AddWireCommand {sf::Vector2f start; sf::Vector2f end;};


struct CircuitCommand {

};

class CircuitService {
private:
    circuitx::Circuit* circuit;
public:
    CircuitService(){}
    virtual ~CircuitService(){}

    void submit(CircuitCommand& cmd);
};



#endif //CIRCUITSERVICE_H
