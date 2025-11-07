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

enum class ComponentRotation : int {
    Right = 0,
    Down = 1,
    Left = 2,
    Up = 3
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

inline int normalizeRotationSteps(int rotationSteps) {
    int normalized = rotationSteps % 4;
    if (normalized < 0) {
        normalized += 4;
    }
    return normalized;
}

inline float rotationStepsToDegrees(int rotationSteps) {
    return static_cast<float>(normalizeRotationSteps(rotationSteps) * 90);
}

inline const char* rotationStepsName(int rotationSteps) {
    switch (normalizeRotationSteps(rotationSteps)) {
        case 0:
            return "Right";
        case 1:
            return "Down";
        case 2:
            return "Left";
        case 3:
            return "Up";
    }
    return "Unknown";
}

#endif //COMPONENTTYPE_H
