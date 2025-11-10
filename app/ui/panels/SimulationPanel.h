//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_SIMULATIONPANEL_H
#define CIRCUITX_SIMULATIONPANEL_H

#include "../../services/CircuitController.h"
#include "../UiState.h"
#include "../UiCommon.h"

class SimulationPanel {
public:
    explicit SimulationPanel(CircuitController& controller);

    void draw(UiState& uiState);

private:
    void drawTransient(UiState& uiState, const TransientResult& transient);

    CircuitController& circuitController;
};

#endif //CIRCUITX_SIMULATIONPANEL_H
