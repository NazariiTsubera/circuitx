//
// Created by Nazarii on 11/1/25.
//

#ifndef GRIDRENDERER_H
#define GRIDRENDERER_H

#include <SFML/Graphics.hpp>

struct GridRenderer {
    sf::VertexArray lines{sf::Lines};
    sf::Color major = {80, 80, 90, 255};
    sf::Color minor = {60, 60, 70, 180};
    float majorStep = 100.f;
    int minorDivisions = 5;



    void update(const sf::View& view) {
        const sf::Vector2f center = view.getCenter();
        const sf::Vector2f size   = view.getSize();
        const float left   = center.x - size.x * 0.5f;
        const float right  = center.x + size.x * 0.5f;
        const float top    = center.y - size.y * 0.5f;
        const float bottom = center.y + size.y * 0.5f;


        lines.clear();
        const float minorStep = majorStep / minorDivisions;
        auto appendLine = [&](sf::Vector2f a, sf::Vector2f b, sf::Color c) {
            lines.append({a, c});
            lines.append({b, c});
        };

        const float startX = std::floor(left / minorStep) * minorStep;
        for (float x = startX; x <= right; x += minorStep) {
            const bool isMajor = fmod(std::abs(x), majorStep) < 0.001f;
            appendLine({x, top}, {x, bottom}, isMajor ? major : minor);
        }

        const float startY = std::floor(top / minorStep) * minorStep;
        for (float y = startY; y <= bottom; y += minorStep) {
            const bool isMajor = fmod(std::abs(y), majorStep) < 0.001f;
            appendLine({left, y}, {right, y}, isMajor ? major : minor);
        }
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        target.draw(lines, states);
    }
};



#endif //GRIDRENDERER_H
