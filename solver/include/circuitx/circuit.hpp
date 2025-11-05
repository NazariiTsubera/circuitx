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

namespace circuitx {
    struct Node {
        unsigned int id;
        std::string name;
    };

    struct Res { unsigned int a; unsigned int b; float res; };
    struct Cap { unsigned int a; unsigned int b; float cap; };
    struct VSource { unsigned int a; unsigned int b; float vol; };
    struct ISource { unsigned int a; unsigned int b; float cur; };
    struct Wire { unsigned int a; unsigned int b; };

    typedef std::variant<Res, Cap, VSource, ISource, Wire> Element;

    class Circuit {
    public:
        Circuit() {}
        virtual ~Circuit() {}

        void addNode(const Node& n) { nodes.push_back(n); }
        void addElement(const Element& e) { elements.push_back(e); }

        Eigen::VectorXd getVector();
        Eigen::MatrixXd getMatrix();

        [[nodiscard]] std::vector<Element> getElements() const { return elements; }
        [[nodiscard]] std::vector<Node> getNodes() const { return nodes; }
        std::vector<Element>& elementsMutable() { return elements; }
        std::vector<Node>& nodesMutable() { return nodes; }
        [[nodiscard]] nlohmann::json toJson() const;
    private:
        std::vector<Node> nodes;
        std::vector<Element> elements;
    };
}

#endif //CIRCUIT_HPP
