//
// Created by Nazarii on 11/2/25.
//

#ifndef CIRCUITCONTROLLER_H
#define CIRCUITCONTROLLER_H

#include "CircuitService.h"
#include "CircuitView.h"

#include <string>

class CircuitController {
public:
    CircuitController(CircuitService& service, CircuitView& view);

    void handle(const CircuitCommand& command);

    CircuitView& getView() { return view; }
    const CircuitView& getView() const { return view; }

    const CircuitService& getService() const { return service; }
    const std::string& getTopology() const { return cachedTopology; }

private:
    void handleWire(const AddWireCommand& command);
    void handleComponent(const AddComponentCommand& command);

private:
    CircuitService& service;
    CircuitView& view;
    std::string cachedTopology;
};

#endif //CIRCUITCONTROLLER_H
