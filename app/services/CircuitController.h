//
// Created by Nazarii on 11/2/25.
//

#ifndef CIRCUITCONTROLLER_H
#define CIRCUITCONTROLLER_H
#include "CircuitService.h"
#include "CircuitView.h"


class CircuitController {
public:
    CircuitController(CircuitService& service, CircuitView &view);
    ~CircuitController();

    const std::string& circuitTopology() const { return topology; }

    void handle(const CircuitCommand& cmd);
private:
    std::string topology;
    CircuitService& service;
    CircuitView& view;
};



#endif //CIRCUITCONTROLLER_H
