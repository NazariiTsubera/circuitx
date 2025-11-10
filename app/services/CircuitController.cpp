//
// Created by Nazarii on 11/2/25.
//

#include "CircuitController.h"

CircuitController::CircuitController(CircuitService& service, CircuitView& view, const CoordinateTool& gridTool)
    : editor(service, view, gridTool) {}

void CircuitController::handle(const CircuitCommand& command) {
    editor.execute(command);
}

void CircuitController::simulate() {
    simulator.runDcAnalysis(editor.getService().getCircuit());
}

void CircuitController::simulateTransient(double durationSeconds, double timestepSeconds) {
    simulator.runTransient(editor.getService(), durationSeconds, timestepSeconds);
}
