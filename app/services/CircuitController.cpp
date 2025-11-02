//
// Created by Nazarii on 11/2/25.
//

#include "CircuitController.h"

#include <nlohmann/json.hpp>

CircuitController::CircuitController(CircuitService &service, CircuitView &view)
    : service(service), view(view)
{
}

CircuitController::~CircuitController() {
}

void CircuitController::handle(const CircuitCommand &cmd) {
    service.submit(cmd);
    topology = service.getCircuit().toJson().dump(2);
}
