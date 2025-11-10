//
// Created by Nazarii on 11/21/25.
//

#include "SettingsPanel.h"

#include <imgui.h>

SettingsPanel::SettingsPanel(StateService& stateService, std::function<void(UiTheme)> onThemeChange)
    : stateService(stateService), onThemeChange(std::move(onThemeChange)) {}

void SettingsPanel::draw(UiState& uiState) {
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
        onThemeChange(UiTheme::Black);
    }
    if (ImGui::RadioButton("Light (Studio)", uiState.theme == UiTheme::White)) {
        onThemeChange(UiTheme::White);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextWrapped("More settings coming soon. Let me know which knobs you need most.");

    ImGui::End();
    if (!open) {
        stateService.setCurrentState(uiState.lastNonSettingsState);
    }
}
