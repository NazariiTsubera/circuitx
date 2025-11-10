//
// Created by Nazarii on 11/21/25.
//

#include "SimulationPanel.h"

#include <imgui.h>

#include <algorithm>
#include <limits>
#include <optional>
#include <string>
#include <vector>

SimulationPanel::SimulationPanel(CircuitController& controller)
    : circuitController(controller) {}

void SimulationPanel::draw(UiState& uiState) {
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
            ImGui::SetTooltip("Copies the plain-text report to the clipboard.");
        }
    }

    ImGui::Separator();

    if (result.nodes.empty()) {
        ImGui::TextUnformatted("No node voltages recorded yet.");
    } else if (ImGui::BeginTable("NodeVoltages", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Node");
        ImGui::TableSetupColumn("Voltage");
        ImGui::TableSetupColumn("Reference");
        ImGui::TableHeadersRow();
        for (const auto& node : result.nodes) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(node.name.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.6f", node.voltage);
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(node.id == result.referenceNodeId ? "Yes" : "");
        }
        ImGui::EndTable();
    }

    ImGui::Separator();

    if (result.elements.empty()) {
        ImGui::TextUnformatted("No element data recorded yet.");
    } else if (ImGui::BeginTable("ElementResults",
                   6,
                   ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Label");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Nodes");
        ImGui::TableSetupColumn("ΔV (V)");
        ImGui::TableSetupColumn("I (A)");
        ImGui::TableSetupColumn("Detail");
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

    drawTransient(uiState, circuitController.fetchTransientResult());

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

void SimulationPanel::drawTransient(UiState& uiState, const TransientResult& transient) {
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
