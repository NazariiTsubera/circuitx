//
// Created by Nazarii on 11/12/25.
//

#ifndef SIMULATIONRESULT_H
#define SIMULATIONRESULT_H

#include <optional>
#include <string>
#include <vector>

#include "ComponentType.h"

struct SimulationNodeResult {
    unsigned int id = 0;
    std::string name;
    double voltage = 0.0;
};

struct SimulationElementResult {
    ComponentType type = ComponentType::Wire;
    std::string label;
    unsigned int nodeA = 0;
    unsigned int nodeB = 0;
    double voltageDrop = 0.0;
    std::optional<double> current;
    std::string detail;
};

struct SimulationResult {
    bool solved = false;
    unsigned int referenceNodeId = 0;
    std::string referenceNodeName = "GND";
    std::string headline;
    std::string textualReport;
    std::vector<SimulationNodeResult> nodes;
    std::vector<SimulationElementResult> elements;

    void clear() {
        solved = false;
        referenceNodeId = 0;
        referenceNodeName = "GND";
        headline.clear();
        textualReport.clear();
        nodes.clear();
        elements.clear();
    }
};

#endif //SIMULATIONRESULT_H
