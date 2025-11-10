//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_CAPACITORSTAMPHANDLER_H
#define CIRCUITX_CAPACITORSTAMPHANDLER_H

#include "../StampContext.h"

namespace circuitx {
    class CapacitorStampHandler : public ElementStampHandler {
    public:
        bool supports(const Element& element) const override;
        void stamp(const Element& element, ElementStampContext& context) const override;
    };
}

#endif //CIRCUITX_CAPACITORSTAMPHANDLER_H
