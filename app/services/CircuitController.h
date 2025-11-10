//
// Created by Nazarii on 11/2/25.
//

#ifndef CIRCUITCONTROLLER_H
#define CIRCUITCONTROLLER_H

#include "../core/CircuitEditor.h"
#include "../core/CircuitSimulator.h"

class CircuitController {
public:
    CircuitController(CircuitService& service, CircuitView& view, const CoordinateTool& gridTool);

    void handle(const CircuitCommand& command);
    void simulate();
    void simulateTransient(double durationSeconds, double timestepSeconds);

    void deleteWire(const WireView& wire) { editor.deleteWire(wire); }
    bool rotateComponent(unsigned int componentId, int rotationDelta) { return editor.rotateComponent(componentId, rotationDelta); }
    std::optional<ComponentView> getComponentAt(sf::Vector2f position, float tolerance = 6.f) const { return editor.getComponentAt(position, tolerance); }
    std::optional<WireView> getWireAt(sf::Vector2f position, float tolerance = 6.f) const { return editor.getWireAt(position, tolerance); }
    std::optional<float> getComponentValue(const ComponentView& component) const { return editor.getComponentValue(component); }
    bool updateComponentValue(const ComponentView& component, float newValue) { return editor.updateComponentValue(component, newValue); }
    void refreshTopologyCache() { editor.refreshTopology(); }
    std::unordered_map<unsigned int, std::string> buildComponentLabels() const { return editor.buildComponentLabels(); }
    std::optional<ComponentView> getComponent(unsigned int componentId) const { return editor.getComponent(componentId); }

    CircuitView& getView() { return editor.getView(); }
    const CircuitView& getView() const { return editor.getView(); }

    CircuitService& getService() { return editor.getService(); }
    const CircuitService& getService() const { return editor.getService(); }

    const std::string& getTopology() const { return editor.topology(); }
    const SimulationResult& fetchSimulationResults() const { return simulator.dcResult(); }
    const TransientResult& fetchTransientResult() const { return simulator.transientResult(); }
    bool hasSelectableAt(sf::Vector2f position) const { return editor.hasSelectableAt(position); }

private:
    CircuitEditor editor;
    CircuitSimulator simulator;
};

#endif //CIRCUITCONTROLLER_H
