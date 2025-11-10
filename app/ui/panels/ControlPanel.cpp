//
// Created by Nazarii on 11/21/25.
//

#include "ControlPanel.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <SFML/System/Vector2.hpp>

ControlPanel::ControlPanel(StateService& stateService, AssetManager& assetManager)
    : stateService(stateService) {
    states.push_back({State::Edit, "Edit", &assetManager.getTexture("edit")});
    states.push_back({State::Play, "Play", &assetManager.getTexture("play")});
    states.push_back({State::Pause, "Pause", &assetManager.getTexture("pause")});
    states.push_back({State::Settings, "Settings", &assetManager.getTexture("gear")});
}

void ControlPanel::draw(const UiState& uiState) {
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
    for (const auto& state : states) {
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
        ImGui::Image(*state.texture, sf::Vector2f(imageSize, imageSize));

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
