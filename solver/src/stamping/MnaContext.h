//
// Created by Nazarii on 11/21/25.
//

#ifndef CIRCUITX_MNACONTEXT_H
#define CIRCUITX_MNACONTEXT_H

#include <unordered_map>
#include <vector>

#include "circuitx/circuit.hpp"

namespace circuitx {

    struct MnaContext {
        unsigned int groundId = 0;
        int nodeUnknowns = 0;
        int voltageUnknowns = 0;
        std::unordered_map<unsigned int, int> nodeToIndex;
        std::unordered_map<std::size_t, int> voltageByElement;
        std::vector<Node> nodesInContext;
        std::vector<unsigned int> indexToNodeId;
        std::vector<std::size_t> voltageIndexToElement;

        [[nodiscard]] int systemSize() const { return nodeUnknowns + voltageUnknowns; }

        [[nodiscard]] int nodeEquationIndex(unsigned int nodeId) const {
            auto it = nodeToIndex.find(nodeId);
            return it == nodeToIndex.end() ? -1 : it->second;
        }

        [[nodiscard]] int voltageEquationIndex(std::size_t elementIndex) const {
            auto it = voltageByElement.find(elementIndex);
            return it == voltageByElement.end() ? -1 : nodeUnknowns + it->second;
        }
    };
}

#endif //CIRCUITX_MNACONTEXT_H
