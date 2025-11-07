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
}

UiService::UiService(
        sf::RenderWindow& window, const Visualizer& visualizer,
        AssetManager& assetManager, const CoordinateTool& gridTool,
        CircuitController& circuitController, StateService& stateService)
    : window(window),
      visualizer(visualizer),
      assetManager(assetManager),
      canvasSize(0.f, 0.f),
      gridTool(gridTool),
      wireTool(gridTool),
      circuitController(circuitController),
      stateService(stateService)
{

    const bool imguiInitialized = ImGui::SFML::Init(window);

    components.push_back({ComponentType::Resistor, "Resistor", assetManager.getTexture("resistor")});
    components.push_back({ComponentType::Capacitor, "Capacitor", assetManager.getTexture("capacitor")});
    components.push_back({ComponentType::ISource, "Current Source", assetManager.getTexture("isource")});
    components.push_back({ComponentType::VSource, "Voltage Source", assetManager.getTexture("vsource")});

    states.push_back({State::Edit, "Edit", assetManager.getTexture("edit")});
    states.push_back({State::Play, "Play", assetManager.getTexture("play")});
    states.push_back({State::Pause, "Pause", assetManager.getTexture("pause")});


    if (!imguiInitialized) {
        throw std::runtime_error("Failed to initialize ImGui-SFML.");
    }

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

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


void UiService::drawCanvas() {

    ImGui::Begin("Canvas");

    if (ImGui::BeginChild("canvas_child", ImVec2(0,0), ImGuiChildFlags_None,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImVec2 availableSize = ImGui::GetContentRegionAvail();

        if (availableSize.x != canvasSize.x || availableSize.y != canvasSize.y) {
            canvasSize = {availableSize.x, availableSize.y};
            canvasTexture.create(static_cast<unsigned int>(canvasSize.x), static_cast<unsigned int>(canvasSize.y));

            sf::View view;
            view.setSize(canvasSize.x, -canvasSize.y);
            view.setCenter(canvasSize.x / 2.f, canvasSize.y / 2.f);

            canvasTexture.setView(view);

            if (resizeCallback) {
                resizeCallback(canvasSize);
            }
        }



        canvasTexture.clear();
        std::optional<WirePreview> preview;

        if (wireTool.isActive()) {
            preview = wireTool.getPreview();
        }

        visualizer.draw(canvasTexture, preview);
        canvasTexture.display();


        // in order to make button overlay
        ImVec2 screenPos = ImGui::GetCursorScreenPos();
        const sf::Vector2f canvasOrigin{screenPos.x, screenPos.y};

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::InvisibleButton("#inv_button", availableSize);
        ImGui::PopStyleVar();
        ImGui::SetCursorScreenPos(screenPos);

        if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            if (ImGui::BeginPopupContextItem("#canvas-context")) {
                const ImVec2 clipMax(screenPos.x + availableSize.x, screenPos.y + availableSize.y);
                ImGui::PushClipRect(screenPos, clipMax, true);
                ImGui::TextUnformatted("Toolbox");

                ImGui::PopClipRect();
                ImGui::EndPopup();
            }
        }

        //Events
        {
            const ImVec2 mousePosIm = ImGui::GetMousePos();
            const sf::Vector2f mousePos = toVector(mousePosIm);
            const sf::Vector2f localMousePos = {mousePos.x - canvasOrigin.x, mousePos.y - canvasOrigin.y};

            const auto componentUnderCursor = circuitController.getComponentAt(localMousePos);
            const auto wireUnderCursor = circuitController.getWireAt(localMousePos);
            const bool selectableUnderCursor = componentUnderCursor.has_value() || wireUnderCursor.has_value();

            if (selectableUnderCursor && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                if (componentUnderCursor) {
                    selectedComponentId = componentUnderCursor->id;
                }
                contextMenuPosition = gridTool.snapToGrid(localMousePos);
                contextMenuComponent = componentUnderCursor;
                contextMenuWire = wireUnderCursor;
                ImGui::OpenPopup("#canvas-context");
            }

            if (ImGui::BeginPopup("#canvas-context")) {
                const ImVec2 clipMax(screenPos.x + availableSize.x, screenPos.y + availableSize.y);
                ImGui::PushClipRect(screenPos, clipMax, true);
                ImGui::TextUnformatted("Toolbox");
                if (contextMenuPosition && ImGui::MenuItem("Delete")) {
                    circuitController.handle(DeleteCommand{*contextMenuPosition});
                    if (propertiesComponent && contextMenuComponent && propertiesComponent->id == contextMenuComponent->id) {
                        showPropertiesWindow = false;
                        propertiesComponent.reset();
                        propertiesStatus.clear();
                    }
                    contextMenuPosition.reset();
                    contextMenuComponent.reset();
                    contextMenuWire.reset();
                    ImGui::CloseCurrentPopup();
                }
                if (contextMenuComponent && componentHasEditableValue(contextMenuComponent->type)) {
                    if (ImGui::MenuItem("Properties")) {
                        auto currentValue = circuitController.getComponentValue(*contextMenuComponent);
                        if (currentValue) {
                            propertiesValue = *currentValue;
                            propertiesStatus.clear();
                        } else {
                            propertiesValue = 0.f;
                            propertiesStatus = "Unable to read current value.";
                        }
                        propertiesComponent = contextMenuComponent;
                        showPropertiesWindow = true;
                        contextMenuPosition.reset();
                        contextMenuComponent.reset();
                        contextMenuWire.reset();
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::PopClipRect();
                ImGui::EndPopup();
            } else {
                contextMenuPosition.reset();
                contextMenuComponent.reset();
                contextMenuWire.reset();
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PALETTE_COMPONENT")) {
                    const auto type = *static_cast<const ComponentType*>(payload->Data);
                    const sf::Vector2f snapped = gridTool.snapToGrid(localMousePos);
                    circuitController.handle(AddComponentCommand{snapped, type, placementRotationSteps});
                }

                ImGui::EndDragDropTarget();
            }

            const bool canvasActive = ImGui::IsItemActive();

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                if (componentUnderCursor) {
                    selectedComponentId = componentUnderCursor->id;
                } else if (!wireUnderCursor) {
                    selectedComponentId.reset();
                }
                wireTool.begin(localMousePos);
            }

            if (canvasActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && wireTool.isActive()) {
                wireTool.update(localMousePos);
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                if (wireTool.isActive()) {
                    const sf::Vector2f origin = wireTool.getOrigin();
                    const sf::Vector2f destination = wireTool.getDestination();
                    wireTool.end();
                    circuitController.handle(AddWireCommand{origin, destination});
                }
            }
        }


        ImGui::Image(canvasTexture.getTexture());

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const auto labelMap = circuitController.buildComponentLabels();
        const SimulationResult& simResult = circuitController.fetchSimulationResults();
        std::optional<ComponentView> selectedComponent;
        if (selectedComponentId) {
            selectedComponent = circuitController.getComponent(*selectedComponentId);
            if (!selectedComponent) {
                selectedComponentId.reset();
            }
        }
        const bool showSimOverlays = simResult.solved && stateService.getCurrentState() != State::Edit;

        auto formatValue = [] (double value) {
            std::ostringstream ss;
            ss.setf(std::ios::fixed, std::ios::floatfield);
            ss.precision(3);
            ss << value;
            return ss.str();
        };

        std::vector<ComponentView> drawableComponents;
        drawableComponents.reserve(circuitController.getView().getComponents().size());
        for (const auto& [componentId, component] : circuitController.getView().getComponents()) {
            if (component.type == ComponentType::Wire) {
                continue;
            }
            drawableComponents.push_back(component);
        }
        std::sort(drawableComponents.begin(), drawableComponents.end(), [](const ComponentView& lhs, const ComponentView& rhs) {
            return lhs.id < rhs.id;
        });

        std::unordered_map<unsigned int, double> nodeVoltages;
        std::unordered_map<unsigned int, std::string> nodeNames;
        std::unordered_map<std::string, const SimulationElementResult*> elementByLabel;

        if (simResult.solved) {
            nodeVoltages.reserve(simResult.nodes.size() + 1);
            nodeNames.reserve(simResult.nodes.size() + 1);
            for (const auto& node : simResult.nodes) {
                nodeVoltages[node.id] = node.voltage;
                nodeNames[node.id] = node.name;
            }
            nodeVoltages[simResult.referenceNodeId] = 0.0;
            nodeNames[simResult.referenceNodeId] = simResult.referenceNodeName;

            elementByLabel.reserve(simResult.elements.size());
            for (const auto& element : simResult.elements) {
                elementByLabel.emplace(element.label, &element);
            }
        }

        if (showSimOverlays) {
            for (const auto& [nodeId, position] : circuitController.getView().getNodes()) {
                const auto voltageIt = nodeVoltages.find(nodeId);
                const auto nameIt = nodeNames.find(nodeId);
                if (voltageIt == nodeVoltages.end() || nameIt == nodeNames.end()) {
                    continue;
                }
                const bool highlightNode = selectedComponent &&
                    (selectedComponent->nodeA == nodeId || selectedComponent->nodeB == nodeId);
                std::string nodeText = nameIt->second + ": " + formatValue(voltageIt->second) + " V";
                const ImVec2 center{canvasOrigin.x + position.x, canvasOrigin.y + position.y};
                const ImVec2 textSize = ImGui::CalcTextSize(nodeText.c_str());
                const float verticalOffset = 26.f;
                const ImVec2 textPos{center.x - textSize.x * 0.5f, center.y - verticalOffset - textSize.y};
                const ImVec2 padding{4.f, 2.f};
                const ImVec2 bgMin{textPos.x - padding.x, textPos.y - padding.y};
                const ImVec2 bgMax{textPos.x + textSize.x + padding.x, textPos.y + textSize.y + padding.y};
                const ImU32 bgColor = highlightNode ? IM_COL32(210, 235, 255, 235) : IM_COL32(245, 245, 245, 220);
                const ImU32 borderColor = highlightNode ? IM_COL32(20, 120, 200, 230) : IM_COL32(60, 60, 60, 200);
                drawList->AddRectFilled(bgMin, bgMax, bgColor);
                drawList->AddRect(bgMin, bgMax, borderColor);
                drawList->AddText(textPos, IM_COL32(20, 20, 20, 255), nodeText.c_str());
            }
        }

        for (const auto& component : drawableComponents) {
            auto it = labelMap.find(component.id);
            if (it == labelMap.end()) {
                continue;
            }
            const bool componentSelected = selectedComponent && selectedComponent->id == component.id;
            std::vector<std::string> lines;
            lines.push_back(it->second);
            if (showSimOverlays) {
                if (auto elemIt = elementByLabel.find(it->second); elemIt != elementByLabel.end() && elemIt->second) {
                    const SimulationElementResult& elem = *elemIt->second;
                    lines.push_back("dV = " + formatValue(elem.voltageDrop) + " V");
                    if (elem.current.has_value()) {
                        lines.push_back("I = " + formatValue(elem.current.value()) + " A");
                    }
                }
            }

            float maxWidth = 0.f;
            float totalHeight = 0.f;
            std::vector<ImVec2> lineSizes;
            lineSizes.reserve(lines.size());
            for (const auto& line : lines) {
                ImVec2 size = ImGui::CalcTextSize(line.c_str());
                lineSizes.push_back(size);
                maxWidth = std::max(maxWidth, size.x);
                totalHeight += size.y;
            }
            const float lineSpacing = 2.f;
            if (lines.size() > 1) {
                totalHeight += lineSpacing * static_cast<float>(lines.size() - 1);
            }

            const ImVec2 center{canvasOrigin.x + component.position.x, canvasOrigin.y + component.position.y};
            const ImVec2 blockSize{maxWidth, totalHeight};
            const float verticalOffset = 32.f;
            const ImVec2 textOrigin{center.x - blockSize.x * 0.5f, center.y - verticalOffset};
            const ImVec2 padding{6.f, 3.f};
            const ImVec2 bgMin{textOrigin.x - padding.x, textOrigin.y - padding.y};
            const ImVec2 bgMax{textOrigin.x + blockSize.x + padding.x, textOrigin.y + blockSize.y + padding.y};

            const ImU32 compBgColor = componentSelected ? IM_COL32(220, 245, 255, 230) : IM_COL32(255, 255, 255, 220);
            const ImU32 compBorderColor = componentSelected ? IM_COL32(10, 110, 190, 230) : IM_COL32(60, 60, 60, 200);

            drawList->AddRectFilled(bgMin, bgMax, compBgColor);
            drawList->AddRect(bgMin, bgMax, compBorderColor);

            float cursorY = textOrigin.y;
            for (std::size_t i = 0; i < lines.size(); ++i) {
                const ImVec2 size = lineSizes[i];
                const float cursorX = center.x - size.x * 0.5f;
                drawList->AddText(ImVec2(cursorX, cursorY), IM_COL32(20, 20, 20, 255), lines[i].c_str());
                cursorY += size.y + lineSpacing;
            }
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

void UiService::drawPalette() {
    ImGui::Begin("Palette");

    ImGui::BeginChild("#pallete_wrapper", ImVec2(0, 0), false,
                      ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding);
    ImGui::BeginTable("Table", 4);

    for (auto& component : components) {
        ImGui::TableNextColumn();
        ImGui::ImageButton(component.texture, ImVec2(50, 50), 3, sf::Color::White);

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ComponentType type = component.type;
            ImGui::SetDragDropPayload("PALETTE_COMPONENT", &type, sizeof(ComponentType));
            ImGui::TextUnformatted(component.name.c_str());
            ImGui::EndDragDropSource();
        }
    }
    ImGui::EndTable();
    ImGui::EndChild();
    ImGui::End();
}

void UiService::drawToolbox() {
    ImGui::Begin("Toolbox");

    ImGui::Text("Placement orientation: %s", rotationStepsName(placementRotationSteps));
    if (ImGui::Button("Rotate placement CCW")) {
        placementRotationSteps = normalizeRotationSteps(placementRotationSteps - 1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Rotate placement CW")) {
        placementRotationSteps = normalizeRotationSteps(placementRotationSteps + 1);
    }

    ImGui::Separator();

    std::optional<ComponentView> selectedComponent;
    if (selectedComponentId) {
        selectedComponent = circuitController.getComponent(*selectedComponentId);
        if (!selectedComponent) {
            selectedComponentId.reset();
        }
    }

    if (selectedComponent) {
        const auto labels = circuitController.buildComponentLabels();
        std::string label;
        if (auto it = labels.find(selectedComponent->id); it != labels.end()) {
            label = it->second;
        }
        ImGui::Text("Selected: %s #%u", componentTypeName(selectedComponent->type), selectedComponent->id);
        if (!label.empty()) {
            ImGui::Text("Label: %s", label.c_str());
        }
        ImGui::Text("Orientation: %s", rotationStepsName(selectedComponent->rotationSteps));

        if (ImGui::Button("Rotate CCW")) {
            if (circuitController.rotateComponent(selectedComponent->id, -1)) {
                toolboxStatus = "Component rotated counter-clockwise.";
                selectedComponent = circuitController.getComponent(selectedComponent->id);
            } else {
                toolboxStatus = "Failed to rotate component.";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Rotate CW")) {
            if (circuitController.rotateComponent(selectedComponent->id, 1)) {
                toolboxStatus = "Component rotated clockwise.";
                selectedComponent = circuitController.getComponent(selectedComponent->id);
            } else {
                toolboxStatus = "Failed to rotate component.";
            }
        }

        if (ImGui::Button("Clear selection")) {
            selectedComponentId.reset();
        }
    } else {
        ImGui::TextUnformatted("No component selected. Click a component on the canvas to select it.");
    }

    if (!toolboxStatus.empty()) {
        ImGui::Separator();
        ImGui::TextUnformatted(toolboxStatus.c_str());
    }

    ImGui::End();
}

void UiService::drawControlPanel() {
    ImGui::Begin("Control panel");

    ImGui::BeginChild("#control_wrapper", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);
    ImGui::BeginTable("#control_table", 10);


    State currentState = stateService.getCurrentState();

    for (auto& state : states) {
        ImGui::TableNextColumn();

        sf::Color color = (state.state == currentState) ? sf::Color::Cyan : sf::Color::White;

        bool clicked = ImGui::ImageButton(state.texture, sf::Vector2f(50, 50), 3, color);

        if (clicked) {
            stateService.setCurrentState(state.state);
        }
    }
    ImGui::EndTable();
    ImGui::EndChild();
    ImGui::End();
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

void UiService::drawPropertiesWindow() {
    if (!showPropertiesWindow || !propertiesComponent) {
        return;
    }

    bool open = showPropertiesWindow;
    const ComponentView& component = *propertiesComponent;

    if (!ImGui::Begin("Component Properties", &open)) {
        ImGui::End();
        if (!open) {
            showPropertiesWindow = false;
            propertiesComponent.reset();
            propertiesStatus.clear();
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
        ImGui::InputFloat(componentValueLabel(component.type), &propertiesValue, 0.0f, 0.0f, "%.6f");
        if (ImGui::Button("Apply")) {
            if (circuitController.updateComponentValue(component, propertiesValue)) {
                propertiesStatus = "Properties updated.";
            } else {
                propertiesStatus = "Failed to update component.";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload")) {
            if (auto current = circuitController.getComponentValue(component)) {
                propertiesValue = *current;
                propertiesStatus.clear();
            } else {
                propertiesStatus = "Unable to reload current value.";
            }
        }
    }

    if (!propertiesStatus.empty()) {
        ImGui::TextUnformatted(propertiesStatus.c_str());
    }

    if (ImGui::Button("Close")) {
        open = false;
    }

    ImGui::End();

    if (!open) {
        showPropertiesWindow = false;
        propertiesComponent.reset();
        propertiesStatus.clear();
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
    drawToolbox();
    drawTopology();
    drawControlPanel();

    if (stateService.getCurrentState() == State::Play) {
        drawSimulation();
    }
    drawPropertiesWindow();
}
