//
// Created by Nazarii on 10/30/25.
//

#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include <string>
namespace circuitx {
    struct ElementDesc {
        std::string label;
    };

    struct Element {
        int id{};
        ElementDesc desc;

        Element* next{};
        Element* prev{};
    };



    struct Source : public ElementDesc {
        float voltage;
    };

    struct Load : public ElementDesc {
        float resitance;
        float capacitance;
    };


    class Circuit {
    private:
        Element* head;
    public:
        Circuit();
        ~Circuit();

        Element* addElement(std::string label, ElementDesc desc, Element* where);
        std::string toJson();
    };
}

#endif //CIRCUIT_HPP
