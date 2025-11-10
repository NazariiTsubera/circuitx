//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_SETTINGSPANEL_H
#define CIRCUITX_SETTINGSPANEL_H

#include "../../services/StateService.h"
#include "../UiState.h"

#include <functional>
#include <utility>

class SettingsPanel {
public:
    SettingsPanel(StateService& stateService, std::function<void(UiTheme)> onThemeChange);

    void draw(UiState& uiState);

private:
    StateService& stateService;
    std::function<void(UiTheme)> onThemeChange;
};

#endif //CIRCUITX_SETTINGSPANEL_H
