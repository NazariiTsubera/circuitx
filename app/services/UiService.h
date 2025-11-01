//
// Created by Nazarii on 11/1/25.
//

#ifndef UISERVICE_H
#define UISERVICE_H
#include "Visualizer.h"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"


class UiService {
private:
    sf::RenderWindow& window;
    sf::RenderTexture canvasTexture;
    const Visualizer& visualizer;
public:
    UiService(sf::RenderWindow& window, const Visualizer& visualizer);
    ~UiService();
    void drawUI();
private:
    void drawCanvas();
    void drawPalette();

    float computePixelScale() const;
};



#endif //UISERVICE_H
