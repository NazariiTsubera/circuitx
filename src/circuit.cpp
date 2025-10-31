//
// Created by Nazarii on 10/30/25.
//

#include "circuitx/circuit.hpp"
namespace circuitx {
    Circuit::Circuit() {
    }

    Circuit::~Circuit() {
    }

    Element * Circuit::addElement(std::string label, ElementDesc desc, Element *where) {
        return nullptr;
    }

    std::string Circuit::toJson() {
        return "";
    }
}
