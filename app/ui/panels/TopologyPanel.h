//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_TOPOLOGYPANEL_H
#define CIRCUITX_TOPOLOGYPANEL_H

#include "../../services/CircuitController.h"

class TopologyPanel {
public:
    explicit TopologyPanel(CircuitController& controller);

    void draw();

private:
    CircuitController& circuitController;
};

#endif //CIRCUITX_TOPOLOGYPANEL_H
