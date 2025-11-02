//
// Created by Nazarii on 11/1/25.
//

#include "UiService.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>
#include <stdexcept>

#include "../helpers/AssetManager.hpp"


UiService::UiService(sf::RenderWindow& window, const Visualizer& visualizer, AssetManager& assetManager, const CoordinateTool& gridTool):
    window(window),
    visualizer(visualizer),
    assetManager(assetManager),
    gridTool(gridTool),
    wireTool(gridTool)
{

    const bool imguiInitialized = ImGui::SFML::Init(window);

    components.push_back({ComponentType::Resistor, "Resistor", assetManager.getTexture("resistor")});
    components.push_back({ComponentType::Capacitor, "Capacitor", assetManager.getTexture("capacitor")});
    components.push_back({ComponentType::ISource, "Current Source", assetManager.getTexture("isource")});
    components.push_back({ComponentType::VSource,"Voltage Source", assetManager.getTexture("vsource")});

    if (!imguiInitialized) {
        throw std::runtime_error("Failed to initialize ImGui-SFML.");
    }

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

    const float pixelScale = computePixelScale();
    if (pixelScale <= 1.0f) {
        return;
    }

    ImFontConfig fontConfig;
    fontConfig.SizePixels = 13.0f * pixelScale;

    io.Fonts->Clear();
    io.Fonts->AddFontDefault(&fontConfig);
    io.DisplayFramebufferScale = ImVec2(pixelScale, pixelScale);

    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(pixelScale);

    if (!ImGui::SFML::UpdateFontTexture()) {
        throw std::runtime_error("Failed to rebuild ImGui font texture for HiDPI scaling.");
    }
}

UiService::~UiService() {
    ImGui::SFML::Shutdown();
}


void UiService::drawCanvas() {

    ImGui::Begin("Canvas");


    ImVec2 availableSize = ImGui::GetContentRegionAvail();

    if (availableSize.x != canvasSize.x || availableSize.y != canvasSize.y) {
        canvasSize = availableSize;
        canvasTexture.create(canvasSize.x, canvasSize.y);

        sf::View view;
        view.setSize(canvasSize.x, -canvasSize.y);
        view.setCenter(canvasSize.x / 2, canvasSize.y / 2);

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
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::InvisibleButton("#inv_button", availableSize);
    ImGui::PopStyleVar();
    ImGui::SetCursorScreenPos(screenPos);

    // sf::Vector2f mousePos = gridTool.screenToWorld(ImGui::GetMousePos());

    sf::Vector2 mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (ImGui::BeginDragDropTarget()) {
        if (auto imguiPayload = ImGui::AcceptDragDropPayload("PALETTE_COMPONENT")) {
            ComponentType type = *static_cast<const ComponentType*>(imguiPayload->Data);
            std::cout << "dropped" << (int)type << std::endl;
        }

        ImGui::EndDragDropTarget();
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        wireTool.begin(mousePos);
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        wireTool.update(mousePos);
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        wireTool.end();
    }


    ImGui::Image(canvasTexture.getTexture());
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
            std::cout << "Drag down" << std::endl;
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
}

