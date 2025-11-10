//
// Created by Nazarii on 11/21/25.
//

#include "VoltageSourceStampHandler.h"

namespace circuitx {
    bool VoltageSourceStampHandler::supports(const Element& element) const {
        return std::holds_alternative<VSource>(element);
    }

    void VoltageSourceStampHandler::stamp(const Element& element, ElementStampContext& context) const {
        const auto& src = std::get<VSource>(element);
        const int eqIdx = context.voltageIndex();
        if (eqIdx < 0) {
            return;
        }
        const int aIdx = context.nodeIndex(src.a);
        const int bIdx = context.nodeIndex(src.b);

        if (context.hasMatrix()) {
            if (aIdx >= 0) {
                context.addToMatrix(aIdx, eqIdx, 1.0);
                context.addToMatrix(eqIdx, aIdx, 1.0);
            }
            if (bIdx >= 0) {
                context.addToMatrix(bIdx, eqIdx, -1.0);
                context.addToMatrix(eqIdx, bIdx, -1.0);
            }
        }

        if (context.hasVector()) {
            context.setVector(eqIdx, static_cast<double>(src.vol));
        }
    }
}
