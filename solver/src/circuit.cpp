//
// Created by Nazarii on 10/30/25.
//

#include "circuitx/circuit.hpp"

#include "stamping/MnaContext.h"
#include "stamping/StampContext.h"

#include <Eigen/Dense>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <memory>
#include <sstream>
#include <type_traits>
#include <unordered_map>

namespace circuitx {
    namespace {

        std::vector<Node> collectNodes(const std::vector<Node>& unitedNodes, const std::vector<Element>& elements) {
            std::vector<Node> nodesForContext;
            nodesForContext.reserve(unitedNodes.size());
            std::unordered_map<unsigned int, std::size_t> indexById;

            auto tryInsertNode = [&](unsigned int id, const std::string& name) {
                if (indexById.find(id) != indexById.end()) {
                    return;
                }
                indexById[id] = nodesForContext.size();
                nodesForContext.push_back(Node{id, name});
            };

            for (const auto& node : unitedNodes) {
                tryInsertNode(node.id, node.name);
            }

            auto ensureNode = [&](unsigned int nodeId) {
                if (indexById.find(nodeId) == indexById.end()) {
                    tryInsertNode(nodeId, "N" + std::to_string(nodeId));
                }
            };

            for (const auto& element : elements) {
                std::visit(
                    [&](const auto& component) {
                        using T = std::decay_t<decltype(component)>;
                        if constexpr (!std::is_same_v<T, Wire>) {
                            ensureNode(component.a);
                            ensureNode(component.b);
                        }
                    },
                    element);
            }

            return nodesForContext;
        }

        unsigned int detectGroundNode(const std::vector<Node>& nodesForContext) {
            if (nodesForContext.empty()) {
                return 0;
            }

            auto idZeroIt = std::find_if(
                nodesForContext.begin(), nodesForContext.end(), [](const Node& node) { return node.id == 0; });
            if (idZeroIt != nodesForContext.end()) {
                return idZeroIt->id;
            }

            auto nameGroundIt = std::find_if(nodesForContext.begin(), nodesForContext.end(), [](const Node& node) {
                std::string lower;
                lower.reserve(node.name.size());
                for (char ch : node.name) {
                    lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
                }
                return lower == "gnd" || lower == "ground";
            });
            if (nameGroundIt != nodesForContext.end()) {
                return nameGroundIt->id;
            }

            auto minIt = std::min_element(nodesForContext.begin(),
                nodesForContext.end(),
                [](const Node& lhs, const Node& rhs) { return lhs.id < rhs.id; });
            return minIt != nodesForContext.end() ? minIt->id : 0;
        }

        MnaContext buildMnaContext(const std::vector<Node>& unitedNodes, const std::vector<Element>& elements) {
            MnaContext ctx;
            ctx.nodesInContext = collectNodes(unitedNodes, elements);
            ctx.groundId = detectGroundNode(ctx.nodesInContext);

            for (const auto& node : ctx.nodesInContext) {
                if (node.id == ctx.groundId) {
                    continue;
                }
                ctx.nodeToIndex[node.id] = ctx.nodeUnknowns++;
                ctx.indexToNodeId.push_back(node.id);
            }

            for (std::size_t idx = 0; idx < elements.size(); ++idx) {
                if (std::holds_alternative<VSource>(elements[idx])) {
                    ctx.voltageByElement[idx] = ctx.voltageUnknowns++;
                    ctx.voltageIndexToElement.push_back(idx);
                }
            }

            return ctx;
        }

        double voltageAtNode(const MnaContext& ctx, unsigned int nodeId, const Eigen::VectorXd& solution) {
            if (nodeId == ctx.groundId) {
                return 0.0;
            }
            const int idx = ctx.nodeEquationIndex(nodeId);
            if (idx < 0 || idx >= solution.size()) {
                return 0.0;
            }
            return solution(idx);
        }

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
    } // namespace

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

    Eigen::VectorXd Circuit::getVector() {
        if (united.empty()) {
            unify();
        }

        const MnaContext ctx = buildMnaContext(united, elements);
        cacheSolutionOrdering(ctx.indexToNodeId, ctx.voltageIndexToElement, ctx.groundId);
        const int totalSize = ctx.systemSize();

        Eigen::VectorXd rhs = Eigen::VectorXd::Zero(totalSize);
        if (totalSize == 0) {
            return rhs;
        }

        ElementStampContext context(ctx, nullptr, &rhs);
        defaultStampRegistry().stamp(elements, context);
        return rhs;
    }

    Eigen::VectorXd Circuit::solve() {
        unify();
        const MnaContext ctx = buildMnaContext(united, elements);
        cacheSolutionOrdering(ctx.indexToNodeId, ctx.voltageIndexToElement, ctx.groundId);
        Eigen::MatrixXd A = Eigen::MatrixXd::Zero(ctx.systemSize(), ctx.systemSize());
        Eigen::VectorXd z = Eigen::VectorXd::Zero(ctx.systemSize());

        if (ctx.systemSize() == 0) {
            return z;
        }

        ElementStampContext context(ctx, &A, &z);
        defaultStampRegistry().stamp(elements, context);

        return A.colPivHouseholderQr().solve(z);
    }

    Eigen::MatrixXd Circuit::getMatrix() {
        if (united.empty()) {
            unify();
        }

        const MnaContext ctx = buildMnaContext(united, elements);
        cacheSolutionOrdering(ctx.indexToNodeId, ctx.voltageIndexToElement, ctx.groundId);
        const int totalSize = ctx.systemSize();

        Eigen::MatrixXd A = Eigen::MatrixXd::Zero(totalSize, totalSize);
        if (totalSize == 0) {
            return A;
        }

        ElementStampContext context(ctx, &A, nullptr);
        defaultStampRegistry().stamp(elements, context);
        return A;
    }

    void Circuit::unify() {
        united.clear();
        if (nodes.empty()) {
            return;
        }

        std::unordered_map<unsigned int, std::size_t> idToIndex;
        idToIndex.reserve(nodes.size());
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            idToIndex[nodes[i].id] = i;
        }

        std::vector<std::size_t> parent(nodes.size());
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            parent[i] = i;
        }

        auto findSet = [&](std::size_t idx) {
            std::size_t root = idx;
            while (parent[root] != root) {
                root = parent[root];
            }
            while (idx != root) {
                std::size_t next = parent[idx];
                parent[idx] = root;
                idx = next;
            }
            return root;
        };

        auto uniteSets = [&](std::size_t a, std::size_t b) {
            std::size_t rootA = findSet(a);
            std::size_t rootB = findSet(b);
            if (rootA == rootB) {
                return;
            }
            if (nodes[rootB].id < nodes[rootA].id) {
                std::swap(rootA, rootB);
            }
            parent[rootB] = rootA;
        };

        for (const auto& element : elements) {
            if (const auto* wire = std::get_if<Wire>(&element)) {
                auto itA = idToIndex.find(wire->a);
                auto itB = idToIndex.find(wire->b);
                if (itA == idToIndex.end() || itB == idToIndex.end()) {
                    continue;
                }
                uniteSets(itA->second, itB->second);
            }
        }

        auto remapNodeId = [&](unsigned int nodeId) {
            auto it = idToIndex.find(nodeId);
            if (it == idToIndex.end()) {
                return nodeId;
            }
            auto root = findSet(it->second);
            return nodes[root].id;
        };

        for (auto& element : elements) {
            std::visit(
                [&](auto& component) {
                    using T = std::decay_t<decltype(component)>;
                    if constexpr (!std::is_same_v<T, Wire>) {
                        component.a = remapNodeId(component.a);
                        component.b = remapNodeId(component.b);
                    }
                },
                element);
        }

        std::vector<bool> added(nodes.size(), false);
        for (std::size_t idx = 0; idx < nodes.size(); ++idx) {
            auto root = findSet(idx);
            if (!added[root]) {
                united.push_back(nodes[root]);
                added[root] = true;
            }
        }
    }

    void Circuit::cacheSolutionOrdering(const std::vector<unsigned int>& nodesInSolution,
        const std::vector<std::size_t>& voltageElements,
        unsigned int groundId) {
        nodeOrdering = nodesInSolution;
        voltageOrdering = voltageElements;
        solutionGroundId = groundId;
    }

    TransientResult Circuit::simulateTransient(double durationSeconds, double timestepSeconds) {
        TransientResult result;
        if (durationSeconds <= 0.0 || timestepSeconds <= 0.0) {
            return result;
        }

        unify();
        const MnaContext ctx = buildMnaContext(united, elements);
        cacheSolutionOrdering(ctx.indexToNodeId, ctx.voltageIndexToElement, ctx.groundId);

        const int systemSize = ctx.systemSize();
        if (systemSize == 0) {
            return result;
        }

        Eigen::MatrixXd baseA = Eigen::MatrixXd::Zero(systemSize, systemSize);
        Eigen::VectorXd baseZ = Eigen::VectorXd::Zero(systemSize);
        std::vector<CapacitorState> capacitors;

        ElementStampContext baseContext(ctx, &baseA, &baseZ, &capacitors);
        defaultStampRegistry().stamp(elements, baseContext);

        Eigen::VectorXd steadyState = solve();

        auto initialVoltageAt = [&](unsigned int nodeId) {
            return voltageAtNode(ctx, nodeId, steadyState);
        };

        for (auto& cap : capacitors) {
            cap.prevVoltage = initialVoltageAt(cap.a) - initialVoltageAt(cap.b);
        }

        const int totalSteps = static_cast<int>(std::ceil(durationSeconds / timestepSeconds));
        result.solved = true;
        result.timestep = timestepSeconds;
        result.referenceNodeId = ctx.groundId;
        result.nodeIds = ctx.indexToNodeId;
        result.nodeVoltages.clear();
        result.nodeVoltages.resize(result.nodeIds.size());
        for (std::size_t i = 0; i < result.nodeIds.size(); ++i) {
            result.nodeIndex[result.nodeIds[i]] = i;
            result.nodeVoltages[i].reserve(static_cast<std::size_t>(totalSteps) + 1);
        }
        // Append ground reference
        result.nodeIndex[result.referenceNodeId] = result.nodeIds.size();
        result.nodeIds.push_back(result.referenceNodeId);
        result.nodeVoltages.emplace_back();
        result.nodeVoltages.back().reserve(static_cast<std::size_t>(totalSteps) + 1);

        auto recordSample = [&](const Eigen::VectorXd& solution, double time) {
            result.times.push_back(time);
            for (std::size_t i = 0; i < ctx.indexToNodeId.size(); ++i) {
                const unsigned int nodeId = ctx.indexToNodeId[i];
                const double voltage = voltageAtNode(ctx, nodeId, solution);
                result.nodeVoltages[i].push_back(voltage);
            }
            // ground
            auto groundIdx = result.nodeIndex[result.referenceNodeId];
            if (groundIdx < result.nodeVoltages.size()) {
                result.nodeVoltages[groundIdx].push_back(0.0);
            }
        };

        recordSample(steadyState, 0.0);

        Eigen::MatrixXd A = Eigen::MatrixXd::Zero(systemSize, systemSize);
        Eigen::VectorXd z = Eigen::VectorXd::Zero(systemSize);

        Eigen::VectorXd currentSolution = steadyState;

        for (int step = 1; step <= totalSteps; ++step) {
            A = baseA;
            z = baseZ;

            for (auto& cap : capacitors) {
                if (cap.capacitance <= 0.0) {
                    continue;
                }
                const double geq = cap.capacitance / timestepSeconds;
                const double historyCurrent = geq * cap.prevVoltage;

                const int aIdx = ctx.nodeEquationIndex(cap.a);
                const int bIdx = ctx.nodeEquationIndex(cap.b);

                if (aIdx >= 0) {
                    A(aIdx, aIdx) += geq;
                    z(aIdx) -= historyCurrent;
                }
                if (bIdx >= 0) {
                    A(bIdx, bIdx) += geq;
                    z(bIdx) += historyCurrent;
                }
                if (aIdx >= 0 && bIdx >= 0) {
                    A(aIdx, bIdx) -= geq;
                    A(bIdx, aIdx) -= geq;
                }
            }

            currentSolution = A.colPivHouseholderQr().solve(z);
            const double currentTime = static_cast<double>(step) * timestepSeconds;
            recordSample(currentSolution, currentTime);

            for (auto& cap : capacitors) {
                const double va = voltageAtNode(ctx, cap.a, currentSolution);
                const double vb = voltageAtNode(ctx, cap.b, currentSolution);
                cap.prevVoltage = va - vb;
            }
        }

        return result;
    }
}
