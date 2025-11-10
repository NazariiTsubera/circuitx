//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_UICOMMON_H
#define CIRCUITX_UICOMMON_H

#include "../services/ComponentType.h"

#include <string>

inline const char* componentTypeName(ComponentType type) {
    switch (type) {
        case ComponentType::Resistor:
            return "Resistor";
        case ComponentType::Capacitor:
            return "Capacitor";
        case ComponentType::ISource:
            return "Current Source";
        case ComponentType::VSource:
            return "Voltage Source";
        case ComponentType::Wire:
            return "Wire";
    }
    return "Unknown";
}

inline const char* componentValueLabel(ComponentType type) {
    switch (type) {
        case ComponentType::Resistor:
            return "Resistance (Ohm)";
        case ComponentType::Capacitor:
            return "Capacitance (F)";
        case ComponentType::ISource:
            return "Current (A)";
        case ComponentType::VSource:
            return "Voltage (V)";
        case ComponentType::Wire:
            return "Value";
    }
    return "Value";
}

inline bool componentHasEditableValue(ComponentType type) {
    return type != ComponentType::Wire;
}

#endif //CIRCUITX_UICOMMON_H
