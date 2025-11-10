//
// Created by Nazarii on 11/1/25.
//

#include "UiService.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <optional>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cfloat>
#include <limits>
#include <cmath>

#include "../helpers/AssetManager.hpp"

namespace {
inline sf::Vector2f toVector(const ImVec2& value) {
    return {value.x, value.y};
}

const char* componentTypeName(ComponentType type) {
    switch (type) {
        case ComponentType::Resistor:
            return "Resistor";
        case ComponentType::Capacitor:
            return "Capacitor";
        case ComponentType::ISource:
            return "Current Source";
        case ComponentType::VSource:
            return "Voltage Source";
        case ComponentType::Wire:
            return "Wire";
    }
    return "Unknown";
}

const char* componentValueLabel(ComponentType type) {
    switch (type) {
        case ComponentType::Resistor:
            return "Resistance (Ohm)";
        case ComponentType::Capacitor:
            return "Capacitance (F)";
        case ComponentType::ISource:
            return "Current (A)";
        case ComponentType::VSource:
            return "Voltage (V)";
        case ComponentType::Wire:
            return "Value";
    }
    return "Value";
}

bool componentHasEditableValue(ComponentType type) {
    return type != ComponentType::Wire;
}

void applyBlackStyle() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 12.f;
    style.FrameRounding = 8.f;
    style.GrabRounding = 8.f;
    style.TabRounding = 8.f;
    style.ScrollbarRounding = 12.f;
    style.ChildBorderSize = 0.f;
    style.FrameBorderSize = 0.f;
    style.WindowBorderSize = 0.f;
    style.PopupBorderSize = 0.f;
    style.WindowPadding = ImVec2(18.f, 18.f);
    style.FramePadding = ImVec2(12.f, 8.f);
    style.ItemSpacing = ImVec2(12.f, 10.f);
    style.ItemInnerSpacing = ImVec2(8.f, 6.f);
    style.IndentSpacing = 18.f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.97f, 1.0f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.59f, 0.66f, 1.0f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.11f, 0.14f, 0.98f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.13f, 0.17f, 0.98f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.17f, 0.19f, 0.24f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.27f, 0.33f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.35f, 0.42f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.13f, 0.24f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.09f, 0.19f, 0.34f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.13f, 0.24f, 0.8f);
    colors[ImGuiCol_Header] = ImVec4(0.21f, 0.26f, 0.36f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.32f, 0.43f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.40f, 0.52f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.21f, 0.25f, 0.33f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.31f, 0.36f, 0.46f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.46f, 0.57f, 1.0f);
    colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.20f, 0.27f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.28f, 0.36f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.33f, 0.42f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(0.32f, 0.38f, 0.50f, 1.0f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.36f, 0.50f, 0.0f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.28f, 0.36f, 0.50f, 0.6f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.28f, 0.36f, 0.50f, 0.9f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.09f, 0.10f, 0.13f, 0.6f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.28f, 0.32f, 0.40f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.36f, 0.40f, 0.50f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.44f, 0.49f, 0.60f, 1.0f);
}

void applyWhiteStyle() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 12.f;
    style.FrameRounding = 8.f;
    style.GrabRounding = 8.f;
    style.TabRounding = 8.f;
    style.ScrollbarRounding = 12.f;
    style.ChildBorderSize = 0.f;
    style.FrameBorderSize = 0.f;
    style.WindowBorderSize = 0.f;
    style.PopupBorderSize = 0.f;
    style.WindowPadding = ImVec2(20.f, 20.f);
    style.FramePadding = ImVec2(12.f, 8.f);
    style.ItemSpacing = ImVec2(12.f, 10.f);
    style.ItemInnerSpacing = ImVec2(8.f, 6.f);
    style.IndentSpacing = 18.f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.12f, 0.14f, 0.22f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.48f, 0.51f, 0.60f, 1.0f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.97f, 0.98f, 1.0f, 1.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.99f, 0.99f, 1.0f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.92f, 0.94f, 0.98f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.86f, 0.89f, 0.96f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.80f, 0.85f, 0.94f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.84f, 0.88f, 0.95f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.77f, 0.83f, 0.94f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.85f, 0.89f, 0.95f, 0.85f);
    colors[ImGuiCol_Header] = ImVec4(0.78f, 0.84f, 0.94f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.79f, 0.93f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.63f, 0.74f, 0.92f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.90f, 0.93f, 0.98f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.78f, 0.85f, 0.96f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.69f, 0.79f, 0.94f, 1.0f);
    colors[ImGuiCol_Tab] = ImVec4(0.90f, 0.93f, 0.97f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.78f, 0.84f, 0.94f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.82f, 0.87f, 0.95f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(0.74f, 0.79f, 0.88f, 1.0f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.66f, 0.75f, 0.92f, 0.0f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.66f, 0.75f, 0.92f, 0.6f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.66f, 0.75f, 0.92f, 0.9f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.94f, 0.96f, 0.99f, 0.7f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.75f, 0.81f, 0.92f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.67f, 0.75f, 0.89f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.60f, 0.70f, 0.87f, 1.0f);
}
}

UiService::UiService(sf::RenderWindow& window,
    Visualizer& visualizer,
    AssetManager& assetManager,
    const CoordinateTool& gridTool,
    CircuitController& circuitController,
    StateService& stateService)
    : window(window),
      visualizer(visualizer),
      assetManager(assetManager),
      canvasSize(0.f, 0.f),
      gridTool(gridTool),
      wireTool(gridTool),
      circuitController(circuitController),
      stateService(stateService),
      canvasPanel(visualizer, circuitController, gridTool, wireTool, stateService)
{

    const bool imguiInitialized = ImGui::SFML::Init(window);

    components.push_back({ComponentType::Resistor, "Resistor", assetManager.getTexture("resistor")});
    components.push_back({ComponentType::Capacitor, "Capacitor", assetManager.getTexture("capacitor")});
    components.push_back({ComponentType::ISource, "Current Source", assetManager.getTexture("isource")});
    components.push_back({ComponentType::VSource, "Voltage Source", assetManager.getTexture("vsource")});

    states.push_back({State::Edit, "Edit", assetManager.getTexture("edit")});
    states.push_back({State::Play, "Play", assetManager.getTexture("play")});
    states.push_back({State::Pause, "Pause", assetManager.getTexture("pause")});
    states.push_back({State::Settings, "Settings", assetManager.getTexture("gear")});

    uiState.lastNonSettingsState = stateService.getCurrentState();
    stateService.addCallback([this](State /*prev*/, State next) {
        if (next != State::Settings) {
            uiState.lastNonSettingsState = next;
        }
    });


    if (!imguiInitialized) {
        throw std::runtime_error("Failed to initialize ImGui-SFML.");
    }

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    applyTheme(uiState.theme);

    const float pixelScale = computePixelScale();
    if (pixelScale > 1.0f) {
        ImFontConfig fontConfig;
        fontConfig.SizePixels = 13.0f * pixelScale;

        io.Fonts->Clear();
        io.Fonts->AddFontDefault(&fontConfig);
        io.DisplayFramebufferScale = ImVec2(pixelScale, pixelScale);

        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(pixelScale);

        if (!ImGui::SFML::UpdateFontTexture()) {
            throw std::runtime_error("Failed to rebuild ImGui font texture for HiDPI scaling.");
        }
    }
}

UiService::~UiService() {
    ImGui::SFML::Shutdown();
}

void UiService::applyTheme(UiTheme theme) {
    uiState.theme = theme;
    if (theme == UiTheme::Black) {
        applyBlackStyle();
        visualizer.setTheme(VisualizerTheme::Dark);
    } else {
        applyWhiteStyle();
        visualizer.setTheme(VisualizerTheme::Light);
    }
}


void UiService::drawCanvas() {
    canvasPanel.draw(uiState, canvasTexture, canvasSize, resizeCallback);
}


void UiService::drawPalette() {
    ImGui::Begin("Palette");

    ImGui::BeginChild("#pallete_wrapper", ImVec2(0, 0), false,
                      ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding);
    ImGui::BeginTable("Table", 4);

    for (auto& component : components) {
        ImGui::TableNextColumn();
        ImGui::PushID(component.name.c_str());
        ImGui::BeginGroup();

        const ImVec2 tileSize{130.f, 150.f};
        ImGui::InvisibleButton("##palette_tile", tileSize);
        const ImVec2 rectMin = ImGui::GetItemRectMin();
        const ImVec2 rectMax = ImGui::GetItemRectMax();
        const ImVec2 cursorAfter = ImGui::GetCursorPos();
        const bool hovered = ImGui::IsItemHovered();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const bool darkTheme = uiState.theme == UiTheme::Black;
        const ImU32 baseColor = darkTheme ? IM_COL32(48, 52, 66, 255) : IM_COL32(235, 237, 243, 255);
        const ImU32 hoverColor = darkTheme ? IM_COL32(64, 70, 88, 255) : IM_COL32(223, 227, 238, 255);
        const ImU32 borderColor = darkTheme ? IM_COL32(112, 122, 150, 255) : IM_COL32(194, 199, 213, 255);
        drawList->AddRectFilled(rectMin, rectMax, hovered ? hoverColor : baseColor, 20.f);
        drawList->AddRect(rectMin, rectMax, borderColor, 20.f, 0, 2.f);

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ComponentType type = component.type;
            ImGui::SetDragDropPayload("PALETTE_COMPONENT", &type, sizeof(ComponentType));
            ImGui::TextUnformatted(component.name.c_str());
            ImGui::EndDragDropSource();
        }

        const float imageSize = 64.f;
        const ImVec2 imagePos{rectMin.x + (tileSize.x - imageSize) * 0.5f, rectMin.y + 18.f};
        ImGui::SetCursorScreenPos(imagePos);
        ImGui::Image(component.texture, sf::Vector2f(imageSize, imageSize));

        ImGui::SetCursorScreenPos(ImVec2(rectMin.x + 14.f, rectMax.y - 44.f));
        ImGui::PushTextWrapPos(rectMax.x - 14.f);
        const ImVec4 tileTextColor = darkTheme ? ImVec4(0.94f, 0.97f, 1.0f, 1.0f)
                                               : ImVec4(0.16f, 0.19f, 0.26f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, tileTextColor);
        ImGui::TextUnformatted(component.name.c_str());
        ImGui::PopStyleColor();
        ImGui::PopTextWrapPos();

        ImGui::SetCursorPos(cursorAfter);
        ImGui::EndGroup();
        ImGui::PopID();
    }
    ImGui::EndTable();
    ImGui::EndChild();
    ImGui::End();
}

void UiService::drawToolbox() {
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


void UiService::drawControlPanel() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(0.f, 190.f), ImVec2(FLT_MAX, 190.f));
    ImGui::Begin("Control panel");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Modes");
    ImGui::BeginChild("#control_wrapper", ImVec2(0, 130.f), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(18.f, 0.f));

    State currentState = stateService.getCurrentState();
    const bool darkTheme = uiState.theme == UiTheme::Black;
    bool first = true;
    for (auto& state : states) {
        if (!first) {
            ImGui::SameLine();
        }
        first = false;

        ImGui::PushID(static_cast<int>(state.state));
        ImGui::BeginGroup();
        const ImVec2 tileSize{130.f, 120.f};
        ImGui::InvisibleButton("##state_tile", tileSize);
        const ImVec2 rectMin = ImGui::GetItemRectMin();
        const ImVec2 rectMax = ImGui::GetItemRectMax();
        const ImVec2 cursorAfter = ImGui::GetCursorPos();
        const bool hovered = ImGui::IsItemHovered();
        const bool active = state.state == currentState;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImU32 inactiveColor = darkTheme
            ? (hovered ? IM_COL32(54, 59, 77, 255) : IM_COL32(44, 48, 63, 255))
            : (hovered ? IM_COL32(226, 231, 241, 255) : IM_COL32(235, 238, 247, 255));
        const ImU32 activeColor = darkTheme
            ? (hovered ? IM_COL32(59, 90, 138, 255) : IM_COL32(48, 78, 122, 255))
            : (hovered ? IM_COL32(208, 224, 250, 255) : IM_COL32(217, 230, 250, 255));
        const ImU32 borderColor = active
            ? (darkTheme ? IM_COL32(130, 176, 233, 255) : IM_COL32(155, 173, 213, 255))
            : (darkTheme ? IM_COL32(90, 100, 126, 255) : IM_COL32(203, 209, 226, 255));
        drawList->AddRectFilled(rectMin, rectMax, active ? activeColor : inactiveColor, 20.f);
        drawList->AddRect(rectMin, rectMax, borderColor, 20.f, 0, 2.5f);

        if (ImGui::IsItemClicked()) {
            stateService.setCurrentState(state.state);
        }

        const float imageSize = 52.f;
        const ImVec2 imagePos{rectMin.x + (tileSize.x - imageSize) * 0.5f, rectMin.y + 14.f};
        ImGui::SetCursorScreenPos(imagePos);
        ImGui::Image(state.texture, sf::Vector2f(imageSize, imageSize));

        ImGui::SetCursorScreenPos(ImVec2(rectMin.x + 12.f, rectMax.y - 40.f));
        ImGui::PushTextWrapPos(rectMax.x - 12.f);
        const ImVec4 modeTextColor = darkTheme ? ImVec4(0.95f, 0.97f, 1.0f, 1.0f)
                                               : ImVec4(0.10f, 0.13f, 0.20f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, modeTextColor);
        ImGui::TextUnformatted(state.name.c_str());
        ImGui::PopStyleColor();
        ImGui::PopTextWrapPos();

        ImGui::SetCursorPos(cursorAfter);
        ImGui::EndGroup();
        ImGui::PopID();
    }

    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::End();
}

void UiService::drawSettingsWindow() {
    if (stateService.getCurrentState() != State::Settings) {
        return;
    }
    bool open = true;
    if (!ImGui::Begin("Settings", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        if (!open) {
            stateService.setCurrentState(uiState.lastNonSettingsState);
        }
        return;
    }

    ImGui::TextUnformatted("Appearance");
    if (ImGui::RadioButton("Dark (Minimal)", uiState.theme == UiTheme::Black)) {
        applyTheme(UiTheme::Black);
    }
    if (ImGui::RadioButton("Light (Studio)", uiState.theme == UiTheme::White)) {
        applyTheme(UiTheme::White);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextWrapped("More settings coming soon. Let me know which knobs you need most.");

    ImGui::End();
    if (!open) {
        stateService.setCurrentState(uiState.lastNonSettingsState);
    }
}

void UiService::drawSimulation() {
    ImGui::Begin("Simulation");

    const SimulationResult& result = circuitController.fetchSimulationResults();

    if (!result.headline.empty()) {
        ImGui::TextWrapped("%s", result.headline.c_str());
    }

    if (!result.textualReport.empty()) {
        if (ImGui::SmallButton("Copy text report")) {
            ImGui::SetClipboardText(result.textualReport.c_str());
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
            ImGui::SetTooltip("Copies the detailed textual summary.");
        }
    }

    if (!result.solved) {
        if (!result.textualReport.empty()) {
            ImGui::Spacing();
            ImGui::TextWrapped("%s", result.textualReport.c_str());
        } else {
            ImGui::Spacing();
            ImGui::TextUnformatted("No simulation data yet.");
        }
        ImGui::End();
        return;
    }

    ImGui::Spacing();
    ImGui::Text("Reference node: %s (ID %u)", result.referenceNodeName.c_str(), result.referenceNodeId);

    if (ImGui::CollapsingHeader("Node Voltages", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (result.nodes.empty()) {
            ImGui::TextUnformatted("Only the reference node is present.");
        } else if (ImGui::BeginTable("node_voltages", 3,
                                      ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Node ID", ImGuiTableColumnFlags_WidthFixed, 70.0f);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Voltage (V)", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            for (const auto& node : result.nodes) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%u", node.id);
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(node.name.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.6f", node.voltage);
            }
            ImGui::EndTable();
        }
    }

    if (ImGui::CollapsingHeader("Element Details", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (result.elements.empty()) {
            ImGui::TextUnformatted("No elements present in the current topology.");
        } else if (ImGui::BeginTable("element_quantities", 6,
                                      ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 110.0f);
            ImGui::TableSetupColumn("Nodes", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("dV (V)", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("I (A)", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            for (const auto& element : result.elements) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(element.label.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(componentTypeName(element.type));
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%u -> %u", element.nodeA, element.nodeB);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.6f", element.voltageDrop);
                ImGui::TableSetColumnIndex(4);
                if (element.current.has_value()) {
                    ImGui::Text("%.6f", element.current.value());
                } else {
                    ImGui::TextUnformatted("-");
                }
                ImGui::TableSetColumnIndex(5);
                ImGui::TextWrapped("%s", element.detail.c_str());
            }
            ImGui::EndTable();
        }
    }

    drawTransientPanel();

    if (!result.textualReport.empty() &&
        ImGui::CollapsingHeader("Raw Text Report")) {
        if (ImGui::BeginChild("#raw_text_report", ImVec2(0, 180.f), true,
                              ImGuiWindowFlags_HorizontalScrollbar)) {
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextUnformatted(result.textualReport.c_str());
            ImGui::PopTextWrapPos();
        }
        ImGui::EndChild();
    }

    ImGui::End();
}

void UiService::drawTransientPanel() {
    auto& transientState = uiState.transient;
    ImGui::Separator();
    ImGui::TextUnformatted("Transient Analysis");
    transientState.duration = std::max(0.0f, transientState.duration);
    transientState.timestep = std::max(1e-6f, transientState.timestep);
    ImGui::InputFloat("Duration (s)", &transientState.duration, 0.0f, 0.0f, "%.6f");
    ImGui::InputFloat("Step (s)", &transientState.timestep, 0.0f, 0.0f, "%.6f");
    ImGui::BeginDisabled(transientState.duration <= 0.0f || transientState.timestep <= 0.0f);
    if (ImGui::Button("Run transient simulation")) {
        circuitController.simulateTransient(transientState.duration, transientState.timestep);
        transientState.selectedNodeIdx = -1;
        transientState.selectedCurrentComponentId = 0;
    }
    ImGui::EndDisabled();

    const TransientResult& transient = circuitController.fetchTransientResult();
    if (transient.times.empty()) {
        ImGui::TextUnformatted("No transient samples available yet.");
        return;
    }

    if (transientState.selectedNodeIdx < 0 && !transient.nodeIds.empty()) {
        transientState.selectedNodeIdx = 0;
    }

    auto nodeLabel = [&](unsigned int nodeId) -> std::string {
        if (nodeId == transient.referenceNodeId) {
            return "GND";
        }
        return "Node " + std::to_string(nodeId);
    };

    if (!transient.nodeIds.empty()) {
        std::string currentLabel = (transientState.selectedNodeIdx >= 0 &&
                                    transientState.selectedNodeIdx < static_cast<int>(transient.nodeIds.size()))
                                        ? nodeLabel(transient.nodeIds[static_cast<std::size_t>(transientState.selectedNodeIdx)])
                                        : "Select node";
        if (ImGui::BeginCombo("Voltage node", currentLabel.c_str())) {
            for (std::size_t idx = 0; idx < transient.nodeIds.size(); ++idx) {
                bool selected = static_cast<int>(idx) == transientState.selectedNodeIdx;
                const std::string label = nodeLabel(transient.nodeIds[idx]);
                if (ImGui::Selectable(label.c_str(), selected)) {
                    transientState.selectedNodeIdx = static_cast<int>(idx);
                }
                if (selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

    auto fetchSeries = [&](unsigned int nodeId) -> const std::vector<double>* {
        auto it = transient.nodeIndex.find(nodeId);
        if (it == transient.nodeIndex.end()) {
            return nullptr;
        }
        const std::size_t index = it->second;
        if (index >= transient.nodeVoltages.size()) {
            return nullptr;
        }
        return &transient.nodeVoltages[index];
    };

    auto plotSeries = [&](const std::vector<double>& samples, std::vector<float>& buffer, const char* label) {
        if (samples.empty()) {
            ImGui::TextUnformatted("No data to plot.");
            return;
        }
        buffer.resize(samples.size());
        float minVal = std::numeric_limits<float>::max();
        float maxVal = std::numeric_limits<float>::lowest();
        for (std::size_t i = 0; i < samples.size(); ++i) {
            float v = static_cast<float>(samples[i]);
            buffer[i] = v;
            minVal = std::min(minVal, v);
            maxVal = std::max(maxVal, v);
        }
        if (minVal == maxVal) {
            maxVal += 1.0f;
            minVal -= 1.0f;
        }
        ImGui::PlotLines(label,
            buffer.data(),
            static_cast<int>(buffer.size()),
            0,
            nullptr,
            minVal,
            maxVal,
            ImVec2(-1, 160));
        ImGui::Text("Samples: %zu  |  dt = %.6f s", samples.size(), transient.timestep);
    };

    if (transientState.selectedNodeIdx >= 0 &&
        transientState.selectedNodeIdx < static_cast<int>(transient.nodeIds.size())) {
        const auto& series = transient.nodeVoltages[static_cast<std::size_t>(transientState.selectedNodeIdx)];
        plotSeries(series, transientState.voltageBuffer, "Voltage (V)");
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Component current");

    std::vector<ComponentView> components;
    components.reserve(circuitController.getView().getComponents().size());
    for (const auto& [id, component] : circuitController.getView().getComponents()) {
        if (component.type == ComponentType::Wire) {
            continue;
        }
        components.push_back(component);
    }
    std::sort(components.begin(), components.end(), [](const ComponentView& lhs, const ComponentView& rhs) {
        return lhs.id < rhs.id;
    });

    if (components.empty()) {
        ImGui::TextUnformatted("Add components to view their currents over time.");
        return;
    }

    if (transientState.selectedCurrentComponentId == 0) {
        transientState.selectedCurrentComponentId = components.front().id;
    }

    const auto labels = circuitController.buildComponentLabels();
    auto componentLabel = [&](const ComponentView& component) -> std::string {
        if (auto it = labels.find(component.id); it != labels.end() && !it->second.empty()) {
            return it->second;
        }
        return std::string(componentTypeName(component.type)) + " #" + std::to_string(component.id);
    };

    std::string componentCurrentLabel = "Select component";
    if (auto comp = circuitController.getComponent(transientState.selectedCurrentComponentId)) {
        componentCurrentLabel = componentLabel(*comp);
    }

    if (ImGui::BeginCombo("Component", componentCurrentLabel.c_str())) {
        for (const auto& component : components) {
            bool selected = component.id == transientState.selectedCurrentComponentId;
            const std::string label = componentLabel(component);
            if (ImGui::Selectable(label.c_str(), selected)) {
                transientState.selectedCurrentComponentId = component.id;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    auto componentForCurrent = circuitController.getComponent(transientState.selectedCurrentComponentId);
    if (!componentForCurrent) {
        ImGui::TextUnformatted("Component not found.");
        return;
    }

    const auto* vaSeries = fetchSeries(componentForCurrent->nodeA);
    const auto* vbSeries = fetchSeries(componentForCurrent->nodeB);
    if (!vaSeries || !vbSeries || vaSeries->size() != vbSeries->size()) {
        ImGui::TextUnformatted("Missing voltage data for component nodes.");
        return;
    }

    const std::size_t sampleCount = std::min(vaSeries->size(), vbSeries->size());
    if (sampleCount == 0) {
        ImGui::TextUnformatted("No samples to display.");
        return;
    }

    const float componentValue = circuitController.getComponentValue(*componentForCurrent).value_or(0.f);
    std::vector<double> currentSamples(sampleCount, 0.0);
    bool supported = true;

    switch (componentForCurrent->type) {
        case ComponentType::Resistor:
            if (componentValue == 0.0f) {
                supported = false;
                uiState.selection.status = "Resistor value is zero.";
                break;
            }
            for (std::size_t i = 0; i < sampleCount; ++i) {
                const double voltage = (*vaSeries)[i] - (*vbSeries)[i];
                currentSamples[i] = voltage / componentValue;
            }
            break;
        case ComponentType::Capacitor:
            if (componentValue == 0.0f || transient.timestep == 0.0) {
                supported = false;
                uiState.selection.status = "Capacitor value or timestep invalid.";
                break;
            }
            for (std::size_t i = 0; i < sampleCount; ++i) {
                const double voltage = (*vaSeries)[i] - (*vbSeries)[i];
                const double prev = i == 0 ? voltage : ((*vaSeries)[i - 1] - (*vbSeries)[i - 1]);
                currentSamples[i] = componentValue * (voltage - prev) / transient.timestep;
            }
            break;
        case ComponentType::ISource:
            for (std::size_t i = 0; i < sampleCount; ++i) {
                currentSamples[i] = componentValue;
            }
            break;
        case ComponentType::VSource:
            supported = false;
            ImGui::TextUnformatted("Voltage source current plotting not supported yet.");
            break;
        case ComponentType::Wire:
            supported = false;
            ImGui::TextUnformatted("Wire currents are not tracked.");
            break;
    }

    if (supported) {
        plotSeries(currentSamples, transientState.currentBuffer, "Current (A)");
    }
}

void UiService::drawPropertiesWindow() {
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

void UiService::drawTopology() {
    ImGui::Begin("Topology");
    ImGui::TextUnformatted(circuitController.getTopology().c_str());
    ImGui::End();
}

float UiService::computePixelScale() const {
    const sf::Vector2u framebufferSize = window.getSize();
    const sf::Vector2f logicalSize = window.getView().getSize();

    if (logicalSize.x <= 0.0f || logicalSize.y <= 0.0f) {
        return 1.0f;
    }

    const float scaleX = static_cast<float>(framebufferSize.x) / logicalSize.x;
    const float scaleY = static_cast<float>(framebufferSize.y) / logicalSize.y;

    return std::max(scaleX, scaleY);
}



void UiService::drawUI() {
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    drawPalette();
    drawCanvas();
    if (uiState.selection.toolboxVisible) {
        drawToolbox();
    } else {
        uiState.selection.toolboxHovered = false;
    }
    drawTopology();
    drawControlPanel();
    drawSettingsWindow();

    if (stateService.getCurrentState() == State::Play) {
        drawSimulation();
    }
    drawPropertiesWindow();

    if (uiState.selection.toolboxVisible &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        !uiState.selection.toolboxHovered) {
        uiState.selection.toolboxVisible = false;
    }
}
