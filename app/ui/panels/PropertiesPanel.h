//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_PROPERTIESPANEL_H
#define CIRCUITX_PROPERTIESPANEL_H

#include "../../services/CircuitController.h"
#include "../UiState.h"
#include "../UiCommon.h"

class PropertiesPanel {
public:
    explicit PropertiesPanel(CircuitController& controller);

    void draw(UiState& uiState);

private:
    CircuitController& circuitController;
};

#endif //CIRCUITX_PROPERTIESPANEL_H
