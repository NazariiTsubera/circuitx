//
// Created by Nazarii on 11/21/25.
//

#include "CurrentSourceStampHandler.h"

namespace circuitx {
    bool CurrentSourceStampHandler::supports(const Element& element) const {
        return std::holds_alternative<ISource>(element);
    }

    void CurrentSourceStampHandler::stamp(const Element& element, ElementStampContext& context) const {
        if (!context.hasVector()) {
            return;
        }
        const auto& src = std::get<ISource>(element);
        const double current = static_cast<double>(src.cur);
        const int aIdx = context.nodeIndex(src.a);
        const int bIdx = context.nodeIndex(src.b);

        if (aIdx >= 0) {
            context.addToVector(aIdx, -current);
        }
        if (bIdx >= 0) {
            context.addToVector(bIdx, current);
        }
    }
}
