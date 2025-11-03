//
// Created by Nazarii on 11/1/25.
//

#ifndef GRIDTOOL_H
#define GRIDTOOL_H
#include <cmath>

#include "SFML/System/Vector2.hpp"

struct GridSettings {
    int spacing;
};

class CoordinateTool {
public:
    explicit CoordinateTool(GridSettings settings) : settings(settings) {}

    sf::Vector2f snapToGrid(sf::Vector2f point) const {
        return sf::Vector2f(
            std::round(point.x / static_cast<float>(settings.spacing)) * settings.spacing,
            std::round(point.y / static_cast<float>(settings.spacing)) * settings.spacing);
    }

    const GridSettings& getSettings() const { return settings; }

private:
    GridSettings settings;
};

#endif //GRIDTOOL_H
