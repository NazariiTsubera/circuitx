//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_CONTROLPANEL_H
#define CIRCUITX_CONTROLPANEL_H

#include "../../helpers/AssetManager.hpp"
#include "../../services/StateService.h"
#include "../UiState.h"

#include <vector>

class ControlPanel {
public:
    ControlPanel(StateService& stateService, AssetManager& assetManager);

    void draw(const UiState& uiState);

private:
    struct StateEntry {
        State state;
        std::string name;
        sf::Texture* texture = nullptr;
    };

    StateService& stateService;
    std::vector<StateEntry> states;
};

#endif //CIRCUITX_CONTROLPANEL_H
