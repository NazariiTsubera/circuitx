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

#include "../helpers/AssetManager.hpp"

namespace {
inline sf::Vector2f toVector(const ImVec2& value) {
    return {value.x, value.y};
}
}

UiService::UiService(sf::RenderWindow& window, const Visualizer& visualizer, AssetManager& assetManager, const CoordinateTool& gridTool, CircuitController& circuitController)
    : window(window),
      canvasTexture(),
      visualizer(visualizer),
      assetManager(assetManager),
      canvasSize(0.f, 0.f),
      components(),
      gridTool(gridTool),
      wireTool(gridTool),
      circuitController(circuitController) {

    const bool imguiInitialized = ImGui::SFML::Init(window);

    components.push_back({ComponentType::Resistor, "Resistor", assetManager.getTexture("resistor")});
    components.push_back({ComponentType::Capacitor, "Capacitor", assetManager.getTexture("capacitor")});
    components.push_back({ComponentType::ISource, "Current Source", assetManager.getTexture("isource")});
    components.push_back({ComponentType::VSource, "Voltage Source", assetManager.getTexture("vsource")});

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

        const bool selectableUnderCursor = circuitController.hasSelectableAt(localMousePos);

        if (selectableUnderCursor) {
            std::cout << "asd";
            circuitController.hasSelectableAt(localMousePos);
        }

        if (selectableUnderCursor && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            contextMenuPosition = gridTool.snapToGrid(localMousePos);
            ImGui::OpenPopup("#canvas-context");
        }

        if (ImGui::BeginPopup("#canvas-context")) {
            const ImVec2 clipMax(screenPos.x + availableSize.x, screenPos.y + availableSize.y);
            ImGui::PushClipRect(screenPos, clipMax, true);
            ImGui::TextUnformatted("Toolbox");
            if (contextMenuPosition && ImGui::MenuItem("Delete")) {
                circuitController.handle(DeleteCommand{*contextMenuPosition});
                contextMenuPosition.reset();
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopClipRect();
            ImGui::EndPopup();
        } else {
            contextMenuPosition.reset();
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PALETTE_COMPONENT")) {
                const auto type = *static_cast<const ComponentType*>(payload->Data);
                const sf::Vector2f snapped = gridTool.snapToGrid(localMousePos);
                circuitController.handle(AddComponentCommand{snapped, type});
            }

            ImGui::EndDragDropTarget();
        }

        const bool canvasActive = ImGui::IsItemActive();

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
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
    drawTopology();
}
