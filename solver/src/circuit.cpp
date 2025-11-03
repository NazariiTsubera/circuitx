//
// Created by Nazarii on 10/30/25.
//

#include "circuitx/circuit.hpp"

#include <nlohmann/json.hpp>
#include <type_traits>

namespace circuitx {

    nlohmann::json serializeElement(const Element& element) {
        return std::visit(
            [](const auto& component) -> nlohmann::json {
                using T = std::decay_t<decltype(component)>;
                if constexpr (std::is_same_v<T, Res>) {
                    return {{"type", "resistor"},
                            {"a", component.a},
                            {"b", component.b},
                            {"value", component.res}};
                } else if constexpr (std::is_same_v<T, Cap>) {
                    return {{"type", "capacitor"},
                            {"a", component.a},
                            {"b", component.b},
                            {"value", component.cap}};
                } else if constexpr (std::is_same_v<T, VSource>) {
                    return {{"type", "voltage_source"},
                            {"a", component.a},
                            {"b", component.b},
                            {"value", component.vol}};
            } else if constexpr (std::is_same_v<T, ISource>) {
                return {{"type", "current_source"},
                        {"a", component.a},
                        {"b", component.b},
                        {"value", component.cur}};
            } else if constexpr (std::is_same_v<T, Wire>) {
                return {{"type", "wire"},
                        {"a", component.a},
                        {"b", component.b}};
            } else {
                return {};
            }
        },
        element);
    }


    nlohmann::json Circuit::toJson() const {
        nlohmann::json nodesJson = nlohmann::json::array();
        for (const auto& node : nodes) {
            nodesJson.push_back({{"id", node.id}, {"name", node.name}});
        }

        nlohmann::json elementsJson = nlohmann::json::array();
        for (const auto& element : elements) {
            elementsJson.push_back(serializeElement(element));
        }

        return {{"nodes", std::move(nodesJson)}, {"elements", std::move(elementsJson)}};
    }

}
