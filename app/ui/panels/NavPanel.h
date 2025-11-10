//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_NAVPANEL_H
#define CIRCUITX_NAVPANEL_H

#include "../../helpers/AssetManager.hpp"
#include "../UiState.h"

class NavPanel {
public:
    explicit NavPanel(AssetManager& assetManager);

    void draw(const UiState& uiState) const;

private:
    sf::Texture* logoTexture = nullptr;
};

#endif //CIRCUITX_NAVPANEL_H
