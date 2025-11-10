//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_VOLTAGESOURCESTAMPHANDLER_H
#define CIRCUITX_VOLTAGESOURCESTAMPHANDLER_H

#include "../StampContext.h"

namespace circuitx {
    class VoltageSourceStampHandler : public ElementStampHandler {
    public:
        bool supports(const Element& element) const override;
        void stamp(const Element& element, ElementStampContext& context) const override;
    };
}

#endif //CIRCUITX_VOLTAGESOURCESTAMPHANDLER_H
