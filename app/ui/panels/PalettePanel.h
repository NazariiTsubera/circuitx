//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_PALETTEPANEL_H
#define CIRCUITX_PALETTEPANEL_H

#include "../../helpers/AssetManager.hpp"
#include "../UiState.h"

#include <vector>

class PalettePanel {
public:
    explicit PalettePanel(AssetManager& assetManager);

    void draw(const UiState& uiState) const;

private:
    struct PaletteEntry {
        ComponentType type;
        std::string name;
        sf::Texture* texture = nullptr;
    };

    std::vector<PaletteEntry> entries;
};

#endif //CIRCUITX_PALETTEPANEL_H
