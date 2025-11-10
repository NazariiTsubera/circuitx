//
// Created by Nazarii on 11/21/25.
//

#include "PalettePanel.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <SFML/System/Vector2.hpp>

PalettePanel::PalettePanel(AssetManager& assetManager) {
    entries.push_back({ComponentType::Resistor, "Resistor", &assetManager.getTexture("resistor")});
    entries.push_back({ComponentType::Capacitor, "Capacitor", &assetManager.getTexture("capacitor")});
    entries.push_back({ComponentType::ISource, "Current Source", &assetManager.getTexture("isource")});
    entries.push_back({ComponentType::VSource, "Voltage Source", &assetManager.getTexture("vsource")});
}

void PalettePanel::draw(const UiState& uiState) const {
    ImGui::Begin("Palette");

    ImGui::BeginChild("#palette_wrapper", ImVec2(0, 0), false,
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding);
    ImGui::BeginTable("PaletteTable", 4);

    const bool darkTheme = uiState.theme == UiTheme::Black;
    for (const auto& entry : entries) {
        ImGui::TableNextColumn();
        ImGui::PushID(entry.name.c_str());
        ImGui::BeginGroup();

        const ImVec2 tileSize{130.f, 150.f};
        ImGui::InvisibleButton("##palette_tile", tileSize);
        const ImVec2 rectMin = ImGui::GetItemRectMin();
        const ImVec2 rectMax = ImGui::GetItemRectMax();
        const ImVec2 cursorAfter = ImGui::GetCursorPos();
        const bool hovered = ImGui::IsItemHovered();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImU32 baseColor = darkTheme ? IM_COL32(48, 52, 66, 255) : IM_COL32(235, 237, 243, 255);
        const ImU32 hoverColor = darkTheme ? IM_COL32(64, 70, 88, 255) : IM_COL32(223, 227, 238, 255);
        const ImU32 borderColor = darkTheme ? IM_COL32(112, 122, 150, 255) : IM_COL32(194, 199, 213, 255);
        drawList->AddRectFilled(rectMin, rectMax, hovered ? hoverColor : baseColor, 20.f);
        drawList->AddRect(rectMin, rectMax, borderColor, 20.f, 0, 2.f);

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ComponentType type = entry.type;
            ImGui::SetDragDropPayload("PALETTE_COMPONENT", &type, sizeof(ComponentType));
            ImGui::TextUnformatted(entry.name.c_str());
            ImGui::EndDragDropSource();
        }

        const float imageSize = 64.f;
        const ImVec2 imagePos{rectMin.x + (tileSize.x - imageSize) * 0.5f, rectMin.y + 18.f};
        ImGui::SetCursorScreenPos(imagePos);
        ImGui::Image(*entry.texture, sf::Vector2f(imageSize, imageSize));

        ImGui::SetCursorScreenPos(ImVec2(rectMin.x + 14.f, rectMax.y - 44.f));
        ImGui::PushTextWrapPos(rectMax.x - 14.f);
        const ImVec4 tileTextColor = darkTheme ? ImVec4(0.94f, 0.97f, 1.0f, 1.0f)
                                               : ImVec4(0.16f, 0.19f, 0.26f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, tileTextColor);
        ImGui::TextUnformatted(entry.name.c_str());
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
