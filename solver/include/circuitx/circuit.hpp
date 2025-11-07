//
// Created by Nazarii on 10/30/25.
//

#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <variant>
#include <vector>
#include <Eigen/Core>
#include <unordered_map>

namespace circuitx {
    struct Node {
        unsigned int id;
        std::string name;
    };



    struct Res { unsigned int a; unsigned int b; float res; };
    struct Cap { unsigned int a; unsigned int b; float cap; };
    struct VSource { unsigned int a; unsigned int b; float vol; };
    struct ISource { unsigned int a; unsigned int b; float cur; };
    struct Wire { unsigned int a; unsigned int b; }; // Exists only to establish connections between nodes (will be merged into supernode at solve time)

    typedef std::variant<Res, Cap, VSource, ISource, Wire> Element;

    struct TransientResult {
        bool solved = false;
        double timestep = 0.0;
        unsigned int referenceNodeId = 0;
        std::vector<double> times;
        std::vector<unsigned int> nodeIds;
        std::vector<std::vector<double>> nodeVoltages;
        std::unordered_map<unsigned int, std::size_t> nodeIndex;
    };

    class Circuit {
    public:
        Circuit() {}
        virtual ~Circuit() {}

        void addNode(const Node& n) { nodes.push_back(n); }
        void addElement(const Element& e) { elements.push_back(e); }

        Eigen::VectorXd solve();
        TransientResult simulateTransient(double durationSeconds, double timestepSeconds);

        [[nodiscard]] std::vector<Element> getElements() const { return elements; }
        [[nodiscard]] std::vector<Node> getNodes() const { return nodes; }
        std::vector<Element>& elementsMutable() { return elements; }
        std::vector<Node>& nodesMutable() { return nodes; }
        [[nodiscard]] nlohmann::json toJson() const;
        [[nodiscard]] const std::vector<unsigned int>& solutionNodeOrdering() const { return nodeOrdering; }
        [[nodiscard]] const std::vector<std::size_t>& solutionVoltageOrdering() const { return voltageOrdering; }
        [[nodiscard]] unsigned int solutionGround() const { return solutionGroundId; }

    private:
        void unify();

        Eigen::VectorXd getVector();
        Eigen::MatrixXd getMatrix();
        void cacheSolutionOrdering(const std::vector<unsigned int>& nodesInSolution,
            const std::vector<std::size_t>& voltageElements,
            unsigned int groundId);
    private:
        std::vector<Node> united;
        std::vector<Node> nodes;
        std::vector<Element> elements;
        std::vector<unsigned int> nodeOrdering;
        std::vector<std::size_t> voltageOrdering;
        unsigned int solutionGroundId = 0;
    };
}

#endif //CIRCUIT_HPP
