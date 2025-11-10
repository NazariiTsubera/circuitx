//
// Created by Nazarii on 11/21/25.
//

#include "PropertiesPanel.h"

#include <imgui.h>

PropertiesPanel::PropertiesPanel(CircuitController& controller)
    : circuitController(controller) {}

void PropertiesPanel::draw(UiState& uiState) {
    auto& properties = uiState.properties;
    if (!properties.showWindow || !properties.component) {
        return;
    }

    bool open = properties.showWindow;
    const ComponentView& component = *properties.component;

    if (!ImGui::Begin("Component Properties", &open)) {
        ImGui::End();
        if (!open) {
            properties.showWindow = false;
            properties.component.reset();
            properties.status.clear();
        }
        return;
    }

    ImGui::Text("Component: %s", componentTypeName(component.type));
    ImGui::Text("ID: %u", component.id);
    ImGui::Text("Nodes: %u -> %u", component.nodeA, component.nodeB);
    if (component.type != ComponentType::Wire) {
        const auto labels = circuitController.buildComponentLabels();
        if (auto it = labels.find(component.id); it != labels.end()) {
            ImGui::Text("Label: %s", it->second.c_str());
        }
    }

    if (!componentHasEditableValue(component.type)) {
        ImGui::TextUnformatted("This component has no editable properties.");
    } else {
        ImGui::InputFloat(componentValueLabel(component.type), &properties.value, 0.0f, 0.0f, "%.6f");
        if (ImGui::Button("Apply")) {
            if (circuitController.updateComponentValue(component, properties.value)) {
                properties.status = "Properties updated.";
            } else {
                properties.status = "Failed to update component.";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload")) {
            if (auto current = circuitController.getComponentValue(component)) {
                properties.value = *current;
                properties.status.clear();
            } else {
                properties.status = "Unable to reload current value.";
            }
        }
    }

    if (!properties.status.empty()) {
        ImGui::TextUnformatted(properties.status.c_str());
    }

    if (ImGui::Button("Close")) {
        open = false;
    }

    ImGui::End();

    if (!open) {
        properties.showWindow = false;
        properties.component.reset();
        properties.status.clear();
    }
}
