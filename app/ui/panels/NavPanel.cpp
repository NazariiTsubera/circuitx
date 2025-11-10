//
// Created by Nazarii on 11/21/25.
//

#include "NavPanel.h"

#include <imgui.h>
#include <imgui-SFML.h>

namespace {
constexpr const char* kMenuItems[] = {"File", "Edit", "View", "Tools", "Window", "Help"};
}

NavPanel::NavPanel(AssetManager& assetManager) {
    try {
        logoTexture = &assetManager.getTexture("logo");
    } catch (...) {
        logoTexture = nullptr;
    }
}

void NavPanel::draw(const UiState&) const {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.f, 16.f));
    if (!ImGui::BeginMainMenuBar()) {
        ImGui::PopStyleVar();
        return;
    }

    if (logoTexture) {
        const float size = 48.f;
        const float available = ImGui::GetFrameHeight();
        const float offset = std::max(0.f, (available - size) * 0.5f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
        ImGui::Image(*logoTexture, ImVec2(size, size));
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
    }

    for (const char* label : kMenuItems) {
        if (ImGui::BeginMenu(label)) {
            ImGui::MenuItem("(Coming soon)", nullptr, false, false);
            ImGui::EndMenu();
        }
    }

    ImGui::EndMainMenuBar();
    ImGui::PopStyleVar();
}
