//
// Created by Nazarii on 11/2/25.
//

#ifndef CIRCUITCONTROLLER_H
#define CIRCUITCONTROLLER_H

#include "CircuitService.h"
#include "CircuitView.h"
#include "../helpers/CoordinateTool.hpp"

#include <string>
#include <vector>

class CircuitController {
public:
    CircuitController(CircuitService& service, CircuitView& view, const CoordinateTool& gridTool);

    void handle(const CircuitCommand& command);

    CircuitView& getView() { return view; }
    const CircuitView& getView() const { return view; }

    const CircuitService& getService() const { return service; }
    const std::string& getTopology() const { return cachedTopology; }
    bool hasSelectableAt(sf::Vector2f position) const { return view.hasSelectableAt(position); }

private:
    void handleWire(const AddWireCommand& command);
    void handleComponent(const AddComponentCommand& command);
    void handleDelete(const DeleteCommand& command);

    unsigned int ensureNodeAt(sf::Vector2f position);
    void addWireSegments(const std::vector<std::pair<unsigned int, sf::Vector2f>>& orderedNodes);
    void cleanupNode(unsigned int nodeId);

private:
    CircuitService& service;
    CircuitView& view;
    const CoordinateTool& gridTool;
    std::string cachedTopology;
};

#endif //CIRCUITCONTROLLER_H
