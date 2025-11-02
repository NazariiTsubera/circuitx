//
// Created by Nazarii on 11/1/25.
//

#include "CircuitService.h"

#include <iostream>
#include <ostream>
#include <nlohmann/json.hpp>

CircuitResult CircuitService::submit(CircuitCommand cmd) {
    std::cout << "submitted" << std::endl;

    if (std::holds_alternative<AddWireCommand>(cmd)) {
        AddWireCommand addWireCommand = std::get<AddWireCommand>(cmd);

        std::cout << "AddWireCommand" << std::endl;
        circuit.addNode(circuitx::Node{nextId, "Node " + std::to_string(nextId)});
        circuit.addNode(circuitx::Node{nextId, "Node " + std::to_string(nextId)});
        nextId++;

        return WireResult{};
    }

    if (std::holds_alternative<DeleteCommand>(cmd)) {
        std::cout << "DeleteCommand" << std::endl;
    }

    if (std::holds_alternative<AddComponentCommand>(cmd)) {
        std::cout << "AddComponentCommand" << std::endl;
    }


    return ErrorResult{"error"};
}


CircuitService::CircuitService() : nextId(0) {
}

CircuitService::~CircuitService() {
}
