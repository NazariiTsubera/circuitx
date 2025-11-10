//
// Created by Nazarii on 11/20/25.
//

#include "CanvasPanel.h"

#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace {
inline sf::Vector2f toVector(const ImVec2& value) {
    return {value.x, value.y};
}
} // namespace

CanvasPanel::CanvasPanel(Visualizer& visualizer,
    CircuitController& circuitController,
    const CoordinateTool& gridTool,
    WireTool& wireTool,
    StateService& stateService)
    : visualizer(visualizer),
      circuitController(circuitController),
      gridTool(gridTool),
      wireTool(wireTool),
      stateService(stateService) {}

void CanvasPanel::draw(UiState& uiState,
    sf::RenderTexture& canvasTexture,
    sf::Vector2f& canvasSize,
    const std::function<void(const sf::Vector2f&)>& resizeCallback) {

    auto& selection = uiState.selection;
    auto& selectedComponentId = selection.selectedComponentId;
    auto& selectedWire = selection.selectedWire;
    auto& selectedNodeId = selection.selectedNodeId;
    auto& selectionKind = selection.selectionKind;

    ImGui::Begin("Canvas");

    if (ImGui::BeginChild("canvas_child",
            ImVec2(0, 0),
            ImGuiChildFlags_None,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        const ImVec2 availableSizeIm = ImGui::GetContentRegionAvail();
        const sf::Vector2f availableSize{availableSizeIm.x, availableSizeIm.y};

        syncCanvasTexture(canvasTexture, canvasSize, availableSize, resizeCallback);

        canvasTexture.clear();
        std::optional<WirePreview> preview;
        if (wireTool.isActive()) {
            preview = wireTool.getPreview();
        }
        visualizer.draw(canvasTexture, preview);
        canvasTexture.display();

        ImVec2 screenPos = ImGui::GetCursorScreenPos();
        const sf::Vector2f canvasOrigin{screenPos.x, screenPos.y};

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::InvisibleButton("#inv_button", availableSizeIm);
        ImGui::PopStyleVar();
        ImGui::SetCursorScreenPos(screenPos);

        auto clearSelection = [&]() {
            selection.clear();
        };

        auto selectComponent = [&](const ComponentView& comp) {
            selectedComponentId = comp.id;
            selectedWire.reset();
            selectedNodeId.reset();
            selectionKind = ToolboxSelection::Component;
        };

        auto selectWire = [&](const WireView& wire) {
            selectedWire = wire;
            selectedComponentId.reset();
            selectedNodeId.reset();
            selectionKind = ToolboxSelection::Wire;
        };

        auto selectNode = [&](unsigned int nodeId) {
            selectedNodeId = nodeId;
            selectedComponentId.reset();
            selectedWire.reset();
            selectionKind = ToolboxSelection::Node;
        };

        auto openToolbox = [&]() {
            if (selectionKind == ToolboxSelection::None) {
                selection.toolboxVisible = false;
                return;
            }
            selection.toolboxVisible = true;
            selection.status.clear();
        };

        {
            const ImVec2 mousePosIm = ImGui::GetMousePos();
            const sf::Vector2f mousePos = toVector(mousePosIm);
            const sf::Vector2f localMousePos = {mousePos.x - canvasOrigin.x, mousePos.y - canvasOrigin.y};

            const auto componentUnderCursor = circuitController.getComponentAt(localMousePos);
            const auto wireUnderCursor = circuitController.getWireAt(localMousePos);
            const auto nodeUnderCursor = circuitController.getView().getNodeNear(localMousePos);

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                if (componentUnderCursor) {
                    selectComponent(*componentUnderCursor);
                    openToolbox();
                } else if (wireUnderCursor) {
                    selectWire(*wireUnderCursor);
                    openToolbox();
                } else if (nodeUnderCursor) {
                    selectNode(*nodeUnderCursor);
                    openToolbox();
                } else {
                    clearSelection();
                }
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PALETTE_COMPONENT")) {
                    const auto type = *static_cast<const ComponentType*>(payload->Data);
                    const sf::Vector2f snapped = gridTool.snapToGrid(localMousePos);
                    circuitController.handle(AddComponentCommand{snapped, type, uiState.placementRotationSteps});
                }
                ImGui::EndDragDropTarget();
            }

            const bool canvasActive = ImGui::IsItemActive();

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                if (componentUnderCursor) {
                    selectComponent(*componentUnderCursor);
                    selection.toolboxVisible = false;
                } else if (wireUnderCursor) {
                    selectWire(*wireUnderCursor);
                    selection.toolboxVisible = false;
                } else if (nodeUnderCursor) {
                    selectNode(*nodeUnderCursor);
                    selection.toolboxVisible = false;
                } else {
                    clearSelection();
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
        const bool darkTheme = uiState.theme == UiTheme::Black;

        auto formatValue = [](double value) {
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

            for (const auto& element : simResult.elements) {
                if (!element.label.empty()) {
                    elementByLabel[element.label] = &element;
                }
            }
        }

        for (const auto& component : drawableComponents) {
            const sf::Vector2f screenPosComponent{component.position.x + canvasOrigin.x, component.position.y + canvasOrigin.y};
            const bool isSelected = selectedComponent && selectedComponent->id == component.id;
            ImU32 outlineColor = isSelected
                ? IM_COL32(255, 255, 255, 255)
                : (darkTheme ? IM_COL32(0, 0, 0, 60) : IM_COL32(0, 0, 0, 120));

            drawList->AddRect({screenPosComponent.x - 20.f, screenPosComponent.y - 12.f},
                {screenPosComponent.x + 20.f, screenPosComponent.y + 12.f},
                outlineColor,
                4.f,
                0,
                2.f);

            if (auto it = labelMap.find(component.id); it != labelMap.end()) {
                const std::string& label = it->second;
                const sf::Vector2f labelPos{screenPosComponent.x - 10.f, screenPosComponent.y - 26.f};
                drawList->AddText({labelPos.x, labelPos.y}, IM_COL32(255, 255, 255, 255), label.c_str());
            }

            if (showSimOverlays) {
                const auto labelIt = labelMap.find(component.id);
                if (labelIt != labelMap.end()) {
                    const auto elemIt = elementByLabel.find(labelIt->second);
                    if (elemIt != elementByLabel.end() && elemIt->second) {
                        const auto* elem = elemIt->second;
                        const std::string line = "ΔV=" + formatValue(elem->voltageDrop) +
                                                 (elem->current ? " V, I=" + formatValue(*elem->current) + " A" : " V");
                        drawList->AddText({screenPosComponent.x - 30.f, screenPosComponent.y + 18.f},
                            darkTheme ? IM_COL32(200, 255, 200, 255) : IM_COL32(10, 80, 10, 255),
                            line.c_str());
                    }
                }
            }
        }

        if (showSimOverlays) {
            for (const auto& [nodeId, position] : circuitController.getView().getNodes()) {
                const auto voltageIt = nodeVoltages.find(nodeId);
                const auto nameIt = nodeNames.find(nodeId);
                if (voltageIt == nodeVoltages.end() || nameIt == nodeNames.end()) {
                    continue;
                }
                const sf::Vector2f screenNodePos{position.x + canvasOrigin.x, position.y + canvasOrigin.y};
                const std::string label = nameIt->second + " = " + formatValue(voltageIt->second) + " V";
                drawList->AddText({screenNodePos.x + 6.f, screenNodePos.y - 6.f},
                    darkTheme ? IM_COL32(180, 220, 255, 255) : IM_COL32(10, 20, 90, 255),
                    label.c_str());
            }
        }
    }

    ImGui::EndChild();
    ImGui::End();
}

void CanvasPanel::syncCanvasTexture(sf::RenderTexture& canvasTexture,
    sf::Vector2f& canvasSize,
    const sf::Vector2f& availableSize,
    const std::function<void(const sf::Vector2f&)>& resizeCallback) {
    if (availableSize.x <= 0.f || availableSize.y <= 0.f) {
        return;
    }

    if (availableSize.x != canvasSize.x || availableSize.y != canvasSize.y) {
        canvasSize = availableSize;
        canvasTexture.create(static_cast<unsigned int>(canvasSize.x), static_cast<unsigned int>(canvasSize.y));

        sf::View view;
        view.setSize(canvasSize.x, -canvasSize.y);
        view.setCenter(canvasSize.x * 0.5f, canvasSize.y * 0.5f);
        canvasTexture.setView(view);

        if (resizeCallback) {
            resizeCallback(canvasSize);
        }
    }
}
