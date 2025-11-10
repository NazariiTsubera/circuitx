//
// Created by Nazarii on 11/21/25.
//

#include "CapacitorStampHandler.h"

namespace circuitx {
    bool CapacitorStampHandler::supports(const Element& element) const {
        return std::holds_alternative<Cap>(element);
    }

    void CapacitorStampHandler::stamp(const Element& element, ElementStampContext& context) const {
        const auto& cap = std::get<Cap>(element);
        context.registerCapacitor(cap.a, cap.b, static_cast<double>(cap.cap));
    }
}
