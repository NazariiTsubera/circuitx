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
    StateService& stateService,
    const std::string& imguiConfigPath)
    : window(window),
      visualizer(visualizer),
      assetManager(assetManager),
      canvasSize(0.f, 0.f),
      gridTool(gridTool),
      wireTool(gridTool),
      circuitController(circuitController),
      stateService(stateService),
      canvasPanel(visualizer, circuitController, gridTool, wireTool, stateService),
      navPanel(assetManager),
      palettePanel(assetManager),
      toolboxPanel(circuitController, gridTool),
      controlPanel(stateService, assetManager),
      simulationPanel(circuitController),
      settingsPanel(stateService, [this](UiTheme theme) { applyTheme(theme); }),
      propertiesPanel(circuitController),
      topologyPanel(circuitController),
      imguiConfigPath(imguiConfigPath)
{

    const bool imguiInitialized = ImGui::SFML::Init(window);

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
    io.IniFilename = imguiConfigPath.c_str();
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
    navPanel.draw(uiState);

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGuiWindowFlags dockFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    if (ImGui::Begin("DockHost", nullptr, dockFlags)) {
        ImGui::DockSpace(ImGui::GetID("CircuitXDockspace"), ImVec2(0.f, 0.f));
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    palettePanel.draw(uiState);
    canvasPanel.draw(uiState, canvasTexture, canvasSize, resizeCallback);
    toolboxPanel.draw(uiState);
    topologyPanel.draw();
    controlPanel.draw(uiState);
    settingsPanel.draw(uiState);

    if (stateService.getCurrentState() == State::Play) {
        simulationPanel.draw(uiState);
    }
    propertiesPanel.draw(uiState);

    if (uiState.selection.toolboxVisible &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        !uiState.selection.toolboxHovered) {
        uiState.selection.toolboxVisible = false;
    }
}
