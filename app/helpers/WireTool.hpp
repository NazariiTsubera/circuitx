//
// Created by Nazarii on 11/1/25.
//

#ifndef WIRETOOL_H
#define WIRETOOL_H
#include "CoordinateTool.hpp"
#include "SFML/System/Vector2.hpp"

struct WirePreview {
    sf::Vector2f origin;
    sf::Vector2f destination;
};



class WireTool {
private:
    sf::Vector2f origin;
    sf::Vector2f destination;
    CoordinateTool gridTool;
    bool active;
public:
    WireTool(CoordinateTool gridTool) : gridTool(gridTool), active(false) {}
    virtual ~WireTool() {}

    void begin(sf::Vector2f position) { origin = gridTool.snapToGrid(position); destination = gridTool.snapToGrid(position); active = true; }
    void update(sf::Vector2f position) { destination = gridTool.snapToGrid(position); }
    void end() { active = false; }

    bool isActive() { return active; }

    sf::Vector2f getOrigin() { return origin; }
    sf::Vector2f getDestination() { return destination; }
    WirePreview getPreview() { return WirePreview{origin, destination}; }
};

#endif //WIRETOOL_H
