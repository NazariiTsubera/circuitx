//
// Created by Nazarii on 11/20/25.
//

#ifndef CIRCUITX_UISTATE_H
#define CIRCUITX_UISTATE_H

#include <optional>
#include <string>
#include <vector>

#include "../services/CircuitView.h"
#include "../services/ComponentType.h"
#include "../services/StateService.h"

enum class UiTheme {
    Black,
    White
};

enum class ToolboxSelection {
    None,
    Component,
    Wire,
    Node
};

struct SelectionState {
    ToolboxSelection selectionKind = ToolboxSelection::None;
    std::optional<unsigned int> selectedComponentId;
    std::optional<WireView> selectedWire;
    std::optional<unsigned int> selectedNodeId;
    bool toolboxVisible = false;
    bool toolboxHovered = false;
    std::string status;

    void clear() {
        selectedComponentId.reset();
        selectedWire.reset();
        selectedNodeId.reset();
        selectionKind = ToolboxSelection::None;
        toolboxVisible = false;
        toolboxHovered = false;
        status.clear();
    }
};

struct PropertiesState {
    bool showWindow = false;
    float value = 0.f;
    std::string status;
    std::optional<ComponentView> component;
};

struct TransientState {
    float duration = 0.01f;
    float timestep = 0.0001f;
    int selectedNodeIdx = -1;
    unsigned int selectedCurrentComponentId = 0;
    std::vector<float> voltageBuffer;
    std::vector<float> currentBuffer;
};

struct UiState {
    UiTheme theme = UiTheme::Black;
    int placementRotationSteps = 0;
    State lastNonSettingsState = State::Edit;

    SelectionState selection;
    PropertiesState properties;
    TransientState transient;
};

#endif //CIRCUITX_UISTATE_H
