//
// Created by Nazarii on 11/20/25.
//

#ifndef CIRCUITX_CIRCUITEDITOR_H
#define CIRCUITX_CIRCUITEDITOR_H

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

#include "../helpers/CoordinateTool.hpp"
#include "../helpers/WireTool.hpp"
#include "../services/CircuitService.h"
#include "../services/CircuitView.h"

class CircuitEditor {
public:
    CircuitEditor(CircuitService& service, CircuitView& view, const CoordinateTool& gridTool);

    void execute(const CircuitCommand& command);
    void deleteWire(const WireView& wire);
    bool rotateComponent(unsigned int componentId, int rotationDelta);

    std::optional<ComponentView> getComponentAt(sf::Vector2f position, float tolerance = 6.f) const;
    std::optional<WireView> getWireAt(sf::Vector2f position, float tolerance = 6.f) const;
    std::optional<ComponentView> getComponent(unsigned int componentId) const;
    std::optional<float> getComponentValue(const ComponentView& component) const;
    bool updateComponentValue(const ComponentView& component, float newValue);
    bool hasSelectableAt(sf::Vector2f position) const;

    std::unordered_map<unsigned int, std::string> buildComponentLabels() const;

    void refreshTopology();
    const std::string& topology() const { return cachedTopology; }

    CircuitView& getView() { return view; }
    const CircuitView& getView() const { return view; }

    CircuitService& getService() { return service; }
    const CircuitService& getService() const { return service; }

private:
    void addWire(const AddWireCommand& command);
    void addComponent(const AddComponentCommand& command);
    void deleteAt(const DeleteCommand& command);

    unsigned int ensureNodeAt(sf::Vector2f position);
    void cleanupNode(unsigned int nodeId);

private:
    CircuitService& service;
    CircuitView& view;
    const CoordinateTool& gridTool;
    std::string cachedTopology;
};

#endif //CIRCUITX_CIRCUITEDITOR_H
