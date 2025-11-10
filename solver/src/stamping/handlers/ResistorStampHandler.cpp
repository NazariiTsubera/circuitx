//
// Created by Nazarii on 11/21/25.
//

#include "ResistorStampHandler.h"

namespace circuitx {
    bool ResistorStampHandler::supports(const Element& element) const {
        return std::holds_alternative<Res>(element);
    }

    void ResistorStampHandler::stamp(const Element& element, ElementStampContext& context) const {
        if (!context.hasMatrix()) {
            return;
        }
        const auto& res = std::get<Res>(element);
        if (res.res <= 0.f) {
            return;
        }

        const double conductance = 1.0 / static_cast<double>(res.res);
        const int aIdx = context.nodeIndex(res.a);
        const int bIdx = context.nodeIndex(res.b);

        if (aIdx >= 0) {
            context.addToMatrix(aIdx, aIdx, conductance);
        }
        if (bIdx >= 0) {
            context.addToMatrix(bIdx, bIdx, conductance);
        }
        if (aIdx >= 0 && bIdx >= 0) {
            context.addToMatrix(aIdx, bIdx, -conductance);
            context.addToMatrix(bIdx, aIdx, -conductance);
        }
    }
}
