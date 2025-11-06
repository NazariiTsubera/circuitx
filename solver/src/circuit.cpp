//
// Created by Nazarii on 10/30/25.
//

#include "circuitx/circuit.hpp"

#include <nlohmann/json.hpp>
#include <type_traits>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <Eigen/Dense>

namespace circuitx {
    namespace {
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

    Eigen::VectorXd Circuit::getVector()
    {
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

        for (std::size_t idx = 0; idx < elements.size(); ++idx) {
            const auto& element = elements[idx];
            if (const auto* isrc = std::get_if<ISource>(&element)) {
                const int aIdx = ctx.nodeEquationIndex(isrc->a);
                const int bIdx = ctx.nodeEquationIndex(isrc->b);
                const double current = static_cast<double>(isrc->cur);

                if (aIdx >= 0) {
                    rhs(aIdx) -= current;
                }
                if (bIdx >= 0) {
                    rhs(bIdx) += current;
                }
            } else if (std::holds_alternative<VSource>(element)) {
                const int eqIdx = ctx.voltageEquationIndex(idx);
                if (eqIdx >= 0) {
                    rhs(eqIdx) = static_cast<double>(std::get<VSource>(element).vol);
                }
            }
        }

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

        // Assemble matrix
        for (std::size_t idx = 0; idx < elements.size(); ++idx) {
            const auto& element = elements[idx];
            if (const auto* res = std::get_if<Res>(&element)) {
                if (res->res <= 0.f) {
                    continue;
                }

                const double conductance = 1.0 / static_cast<double>(res->res);
                const int aIdx = ctx.nodeEquationIndex(res->a);
                const int bIdx = ctx.nodeEquationIndex(res->b);

                if (aIdx >= 0) {
                    A(aIdx, aIdx) += conductance;
                }
                if (bIdx >= 0) {
                    A(bIdx, bIdx) += conductance;
                }
                if (aIdx >= 0 && bIdx >= 0) {
                    A(aIdx, bIdx) -= conductance;
                    A(bIdx, aIdx) -= conductance;
                }
            } else if (const auto* vsrc = std::get_if<VSource>(&element)) {
                const int aIdx = ctx.nodeEquationIndex(vsrc->a);
                const int bIdx = ctx.nodeEquationIndex(vsrc->b);
                const int eqIdx = ctx.voltageEquationIndex(idx);

                if (eqIdx < 0) {
                    continue;
                }

                if (aIdx >= 0) {
                    A(aIdx, eqIdx) += 1.0;
                    A(eqIdx, aIdx) += 1.0;
                }
                if (bIdx >= 0) {
                    A(bIdx, eqIdx) -= 1.0;
                    A(eqIdx, bIdx) -= 1.0;
                }
            } else if (std::holds_alternative<Cap>(element)) {
                // Capacitor stamping requires discretization (dt) which is not yet implemented.
            }
        }

        // Assemble vector
        for (std::size_t idx = 0; idx < elements.size(); ++idx) {
            const auto& element = elements[idx];
            if (const auto* isrc = std::get_if<ISource>(&element)) {
                const int aIdx = ctx.nodeEquationIndex(isrc->a);
                const int bIdx = ctx.nodeEquationIndex(isrc->b);
                const double current = static_cast<double>(isrc->cur);

                if (aIdx >= 0) {
                    z(aIdx) -= current;
                }
                if (bIdx >= 0) {
                    z(bIdx) += current;
                }
            } else if (const auto* vsrc = std::get_if<VSource>(&element)) {
                const int eqIdx = ctx.voltageEquationIndex(idx);
                if (eqIdx >= 0) {
                    z(eqIdx) = static_cast<double>(vsrc->vol);
                }
            }
        }

        return A.colPivHouseholderQr().solve(z);
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


    Eigen::MatrixXd Circuit::getMatrix()
    {
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

        for (std::size_t idx = 0; idx < elements.size(); ++idx) {
            const auto& element = elements[idx];
            if (const auto* res = std::get_if<Res>(&element)) {
                if (res->res <= 0.f) {
                    continue;
                }

                const double conductance = 1.0 / static_cast<double>(res->res);
                const int aIdx = ctx.nodeEquationIndex(res->a);
                const int bIdx = ctx.nodeEquationIndex(res->b);

                if (aIdx >= 0) {
                    A(aIdx, aIdx) += conductance;
                }
                if (bIdx >= 0) {
                    A(bIdx, bIdx) += conductance;
                }
                if (aIdx >= 0 && bIdx >= 0) {
                    A(aIdx, bIdx) -= conductance;
                    A(bIdx, aIdx) -= conductance;
                }
            } else if (const auto* vsrc = std::get_if<VSource>(&element)) {
                const int aIdx = ctx.nodeEquationIndex(vsrc->a);
                const int bIdx = ctx.nodeEquationIndex(vsrc->b);
                const int eqIdx = ctx.voltageEquationIndex(idx);

                if (eqIdx < 0) {
                    continue;
                }

                if (aIdx >= 0) {
                    A(aIdx, eqIdx) += 1.0;
                    A(eqIdx, aIdx) += 1.0;
                }
                if (bIdx >= 0) {
                    A(bIdx, eqIdx) -= 1.0;
                    A(eqIdx, bIdx) -= 1.0;
                }
            } else if (std::holds_alternative<Cap>(element)) {
                //TODO: implement time stamping
            }
        }

        return A;
    }
}
