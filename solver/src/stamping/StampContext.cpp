//
// Created by Nazarii on 11/21/25.
//

#include "StampContext.h"

#include "handlers/ResistorStampHandler.h"
#include "handlers/VoltageSourceStampHandler.h"
#include "handlers/CurrentSourceStampHandler.h"
#include "handlers/CapacitorStampHandler.h"

namespace circuitx {

    ElementStampContext::ElementStampContext(const MnaContext& ctx,
        Eigen::MatrixXd* matrix,
        Eigen::VectorXd* rhs,
        std::vector<CapacitorState>* capacitorCollector)
        : ctx(ctx),
          matrix(matrix),
          rhs(rhs),
          capacitorCollector(capacitorCollector) {}

    void ElementStampContext::addToMatrix(int row, int col, double value) {
        if (!matrix || row < 0 || col < 0) {
            return;
        }
        (*matrix)(row, col) += value;
    }

    void ElementStampContext::addToVector(int row, double value) {
        if (!rhs || row < 0) {
            return;
        }
        (*rhs)(row) += value;
    }

    void ElementStampContext::setVector(int row, double value) {
        if (!rhs || row < 0) {
            return;
        }
        (*rhs)(row) = value;
    }

    void ElementStampContext::registerCapacitor(unsigned int a, unsigned int b, double capacitance) {
        if (!capacitorCollector || capacitance <= 0.0) {
            return;
        }
        capacitorCollector->push_back(CapacitorState{a, b, capacitance, 0.0});
    }

    void ElementStampRegistry::stamp(const std::vector<Element>& elements, ElementStampContext& context) const {
        for (std::size_t idx = 0; idx < elements.size(); ++idx) {
            context.setCurrentElementIndex(idx);
            for (const auto& handler : handlers) {
                if (handler->supports(elements[idx])) {
                    handler->stamp(elements[idx], context);
                    break;
                }
            }
        }
    }

    const ElementStampRegistry& defaultStampRegistry() {
        static ElementStampRegistry registry = [] {
            ElementStampRegistry reg;
            reg.addHandler<ResistorStampHandler>();
            reg.addHandler<VoltageSourceStampHandler>();
            reg.addHandler<CurrentSourceStampHandler>();
            reg.addHandler<CapacitorStampHandler>();
            return reg;
        }();
        return registry;
    }
}
