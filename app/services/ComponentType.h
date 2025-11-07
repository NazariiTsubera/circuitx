//
// Created by Nazarii on 11/2/25.
//

#ifndef COMPONENTTYPE_H
#define COMPONENTTYPE_H

#include <cstddef>
#include <string>

enum class ComponentType {
    Resistor = 0,
    Capacitor = 1,
    ISource = 2,
    VSource = 3,
    Wire = 4
};

inline const char* componentLabelPrefix(ComponentType type) {
    switch (type) {
        case ComponentType::Resistor:
            return "R";
        case ComponentType::Capacitor:
            return "C";
        case ComponentType::ISource:
            return "I";
        case ComponentType::VSource:
            return "V";
        case ComponentType::Wire:
            return "";
    }
    return "";
}

inline std::string componentLabel(ComponentType type, std::size_t index) {
    const char* prefix = componentLabelPrefix(type);
    if (!prefix || prefix[0] == '\0') {
        return {};
    }
    return std::string(prefix) + std::to_string(index);
}

#endif //COMPONENTTYPE_H
