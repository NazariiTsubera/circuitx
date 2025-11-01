//
// Created by Nazarii on 10/30/25.
//

#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include <string>
#include <variant>
#include <vector>

namespace circuitx {
    struct Node {
        unsigned int id;
        std::string name;
    };

    struct Res { unsigned int a; unsigned int b; float res; };
    struct Cap { unsigned int a; unsigned int b; float cap; };
    struct VSource { unsigned int a; unsigned int b; float vol; };
    struct ISource { unsigned int a; unsigned int b; float cur; };

    typedef std::variant<Res, Cap, VSource, ISource> Element;

    class Circuit {
    public:
        Circuit() {}
        virtual ~Circuit() {}

        void addNode(const Node& n) { nodes.push_back(n); }
        void addElement(const Element& e) { elements.push_back(e); }

        [[nodiscard]] std::vector<Element> getElements() const { return elements; }
        [[nodiscard]] std::vector<Node> getNodes() const { return nodes; }
    private:
        std::vector<Node> nodes;
        std::vector<Element> elements;
    };
}

#endif //CIRCUIT_HPP
