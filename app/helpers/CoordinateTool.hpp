//
// Created by Nazarii on 11/1/25.
//

#ifndef GRIDTOOL_H
#define GRIDTOOL_H
#include "SFML/Audio/InputSoundFile.hpp"
#include "SFML/System/Vector2.hpp"

struct GridSettings {
    int spacing;
};

class CoordinateTool {
public:
    CoordinateTool(GridSettings settings) { this->settings = settings; }


    sf::Vector2f snapToGrid(sf::Vector2f point) const {
        return sf::Vector2f(
            roundf(point.x / settings.spacing) * settings.spacing,
            roundf(point.y / settings.spacing) * settings.spacing);
    }

    GridSettings getSettings() { return settings; }
private:
    GridSettings settings;
};

#endif //GRIDTOOL_H
