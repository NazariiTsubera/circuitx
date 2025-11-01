//
// Created by Nazarii on 11/1/25.
//

#include "UiService.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <stdexcept>


UiService::UiService(sf::RenderWindow& window, const Visualizer& visualizer):
    window(window),
    visualizer(visualizer)
{
    canvasTexture.create(window.getSize().x, window.getSize().y);
    const bool imguiInitialized = ImGui::SFML::Init(window);

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
    canvasTexture.clear();
    visualizer.draw(canvasTexture);
    canvasTexture.display();

    ImGui::Begin("Canvas");
    ImGui::Image(canvasTexture.getTexture());
    ImGui::End();
}

void UiService::drawPalette() {
    ImGui::Begin("Palette");

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