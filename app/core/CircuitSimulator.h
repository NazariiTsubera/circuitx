//
// Created by Nazarii on 11/20/25.
//

#ifndef CIRCUITX_CIRCUITSIMULATOR_H
#define CIRCUITX_CIRCUITSIMULATOR_H

#include "../services/CircuitService.h"
#include "../services/SimulationResult.h"

class CircuitSimulator {
public:
    CircuitSimulator();

    void runDcAnalysis(circuitx::Circuit circuit);
    void runTransient(CircuitService& service, double durationSeconds, double timestepSeconds);

    const SimulationResult& dcResult() const { return simulationResult; }
    const TransientResult& transientResult() const { return transientSimulation; }

private:
    SimulationResult simulationResult;
    TransientResult transientSimulation;
};

#endif //CIRCUITX_CIRCUITSIMULATOR_H
