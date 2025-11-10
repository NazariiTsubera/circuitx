//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_TOOLBOXPANEL_H
#define CIRCUITX_TOOLBOXPANEL_H

#include "../../helpers/CoordinateTool.hpp"
#include "../../ui/UiState.h"
#include "../../ui/UiCommon.h"
#include "../../services/CircuitController.h"

class ToolboxPanel {
public:
    ToolboxPanel(CircuitController& controller, const CoordinateTool& gridTool);

    void draw(UiState& uiState);

private:
    CircuitController& circuitController;
    const CoordinateTool& gridTool;
};

#endif //CIRCUITX_TOOLBOXPANEL_H
