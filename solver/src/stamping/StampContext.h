//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_STAMPCONTEXT_H
#define CIRCUITX_STAMPCONTEXT_H

#include "MnaContext.h"

#include <Eigen/Dense>

#include <memory>
#include <vector>

namespace circuitx {
    struct CapacitorState {
        unsigned int a = 0;
        unsigned int b = 0;
        double capacitance = 0.0;
        double prevVoltage = 0.0;
    };

    class ElementStampContext {
    public:
        ElementStampContext(const MnaContext& ctx,
            Eigen::MatrixXd* matrix,
            Eigen::VectorXd* rhs,
            std::vector<CapacitorState>* capacitorCollector = nullptr);

        void setCurrentElementIndex(std::size_t idx) { currentElementIndex = idx; }

        [[nodiscard]] int nodeIndex(unsigned int nodeId) const { return ctx.nodeEquationIndex(nodeId); }
        [[nodiscard]] int voltageIndex() const { return ctx.voltageEquationIndex(currentElementIndex); }
        [[nodiscard]] bool hasMatrix() const { return matrix != nullptr; }
        [[nodiscard]] bool hasVector() const { return rhs != nullptr; }

        void addToMatrix(int row, int col, double value);
        void addToVector(int row, double value);
        void setVector(int row, double value);
        void registerCapacitor(unsigned int a, unsigned int b, double capacitance);

    private:
        const MnaContext& ctx;
        Eigen::MatrixXd* matrix;
        Eigen::VectorXd* rhs;
        std::vector<CapacitorState>* capacitorCollector;
        std::size_t currentElementIndex = 0;
    };

    class ElementStampHandler {
    public:
        virtual ~ElementStampHandler() = default;
        virtual bool supports(const Element& element) const = 0;
        virtual void stamp(const Element& element, ElementStampContext& context) const = 0;
    };

    class ElementStampRegistry {
    public:
        template<typename Handler, typename... Args>
        void addHandler(Args&&... args) {
            handlers.emplace_back(std::make_unique<Handler>(std::forward<Args>(args)...));
        }

        void stamp(const std::vector<Element>& elements, ElementStampContext& context) const;

    private:
        std::vector<std::unique_ptr<ElementStampHandler>> handlers;
    };

    const ElementStampRegistry& defaultStampRegistry();
}

#endif //CIRCUITX_STAMPCONTEXT_H
