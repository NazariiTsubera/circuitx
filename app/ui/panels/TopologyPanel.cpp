//
// Created by Nazarii on 11/21/25.
//

#include "TopologyPanel.h"

#include <imgui.h>

TopologyPanel::TopologyPanel(CircuitController& controller)
    : circuitController(controller) {}

void TopologyPanel::draw() {
    ImGui::Begin("Topology");
    ImGui::TextUnformatted(circuitController.getTopology().c_str());
    ImGui::End();
}
