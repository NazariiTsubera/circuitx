//
// Created by Nazarii on 11/20/25.
//

#ifndef CIRCUITX_CANVASPANEL_H
#define CIRCUITX_CANVASPANEL_H

#include <functional>
#include <optional>

#include <SFML/Graphics/RenderTexture.hpp>

#include "../helpers/WireTool.hpp"
#include "../services/CircuitController.h"
#include "../services/StateService.h"
#include "../services/Visualizer.h"
#include "UiState.h"

class CanvasPanel {
public:
    CanvasPanel(Visualizer& visualizer,
        CircuitController& circuitController,
        const CoordinateTool& gridTool,
        WireTool& wireTool,
        StateService& stateService);

    void draw(UiState& uiState,
        sf::RenderTexture& canvasTexture,
        sf::Vector2f& canvasSize,
        const std::function<void(const sf::Vector2f&)>& resizeCallback);

private:
    void syncCanvasTexture(sf::RenderTexture& canvasTexture,
        sf::Vector2f& canvasSize,
        const sf::Vector2f& availableSize,
        const std::function<void(const sf::Vector2f&)>& resizeCallback);

private:
    Visualizer& visualizer;
    CircuitController& circuitController;
    const CoordinateTool& gridTool;
    WireTool& wireTool;
    StateService& stateService;
};

#endif //CIRCUITX_CANVASPANEL_H
