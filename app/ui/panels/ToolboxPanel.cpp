//
// Created by Nazarii on 11/21/25.
//

#include "ToolboxPanel.h"

#include <imgui.h>

ToolboxPanel::ToolboxPanel(CircuitController& controller, const CoordinateTool& gridTool)
    : circuitController(controller), gridTool(gridTool) {}

void ToolboxPanel::draw(UiState& uiState) {
    auto& selection = uiState.selection;
    auto& properties = uiState.properties;

    if (!selection.toolboxVisible) {
        selection.toolboxHovered = false;
        return;
    }

    if (selection.selectionKind == ToolboxSelection::None) {
        selection.toolboxHovered = false;
        selection.toolboxVisible = false;
        return;
    }

    bool open = selection.toolboxVisible;
    if (!ImGui::Begin("Toolbox", &open)) {
        selection.toolboxHovered = false;
        ImGui::End();
        selection.toolboxVisible = open;
        return;
    }

    selection.toolboxHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup |
        ImGuiHoveredFlags_ChildWindows);

    auto clearSelection = [&]() {
        selection.clear();
    };

    auto deleteAtPosition = [&](const sf::Vector2f& position) {
        const sf::Vector2f snapped = gridTool.snapToGrid(position);
        circuitController.handle(DeleteCommand{snapped});
        selection.status = "Element deleted.";
        if (properties.component) {
            properties.showWindow = false;
            properties.component.reset();
            properties.status.clear();
        }
        clearSelection();
    };

    ImGui::Text("Placement orientation: %s", rotationStepsName(uiState.placementRotationSteps));
    if (ImGui::Button("Rotate placement CCW")) {
        uiState.placementRotationSteps = normalizeRotationSteps(uiState.placementRotationSteps - 1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Rotate placement CW")) {
        uiState.placementRotationSteps = normalizeRotationSteps(uiState.placementRotationSteps + 1);
    }

    ImGui::Separator();

    if (selection.selectionKind == ToolboxSelection::Component && selection.selectedComponentId) {
        if (auto selectedComponent = circuitController.getComponent(*selection.selectedComponentId)) {
            const auto labels = circuitController.buildComponentLabels();
            std::string label;
            if (auto it = labels.find(selectedComponent->id); it != labels.end()) {
                label = it->second;
            }
            ImGui::Text("Component: %s (#%u)", componentTypeName(selectedComponent->type), selectedComponent->id);
            if (!label.empty()) {
                ImGui::Text("Label: %s", label.c_str());
            }
            ImGui::Text("Orientation: %s", rotationStepsName(selectedComponent->rotationSteps));

            if (ImGui::Button("Rotate CCW")) {
                if (circuitController.rotateComponent(selectedComponent->id, -1)) {
                    selection.status = "Component rotated counter-clockwise.";
                } else {
                    selection.status = "Failed to rotate component.";
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Rotate CW")) {
                if (circuitController.rotateComponent(selectedComponent->id, 1)) {
                    selection.status = "Component rotated clockwise.";
                } else {
                    selection.status = "Failed to rotate component.";
                }
            }

            if (componentHasEditableValue(selectedComponent->type)) {
                if (ImGui::Button("Edit Value")) {
                    if (auto current = circuitController.getComponentValue(*selectedComponent)) {
                        properties.value = *current;
                        properties.status.clear();
                    } else {
                        properties.value = 0.f;
                        properties.status = "Unable to read current value.";
                    }
                    properties.component = selectedComponent;
                    properties.showWindow = true;
                }
            }

            if (ImGui::Button("Delete Component")) {
                deleteAtPosition(selectedComponent->position);
                ImGui::End();
                selection.toolboxHovered = false;
                return;
            }
        } else {
            clearSelection();
        }
    } else if (selection.selectionKind == ToolboxSelection::Wire && selection.selectedWire) {
        ImGui::Text("Wire: %u -> %u", selection.selectedWire->startNode, selection.selectedWire->endNode);
        if (ImGui::Button("Delete Wire")) {
            circuitController.deleteWire(*selection.selectedWire);
            circuitController.getTopology();
            clearSelection();
            ImGui::End();
            selection.toolboxHovered = false;
            return;
        }
    } else if (selection.selectionKind == ToolboxSelection::Node && selection.selectedNodeId) {
        const auto nodePos = circuitController.getView().getNodePosition(*selection.selectedNodeId);
        ImGui::Text("Node: %u", *selection.selectedNodeId);
        if (nodePos) {
            ImGui::Text("Position: (%.1f, %.1f)", nodePos->x, nodePos->y);
        }
        ImGui::TextWrapped("Nodes inherit their behavior from connected elements. Delete or edit those elements instead.");
        if (ImGui::Button("Deselect")) {
            clearSelection();
            ImGui::End();
            selection.toolboxHovered = false;
            return;
        }
    } else {
        ImGui::TextUnformatted("Select an element, wire, or node to inspect.");
    }

    if (!selection.status.empty()) {
        ImGui::Separator();
        ImGui::TextUnformatted(selection.status.c_str());
    }

    ImGui::End();
    if (!open) {
        clearSelection();
    } else {
        selection.toolboxVisible = true;
    }
}
