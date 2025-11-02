//
// Created by Nazarii on 11/1/25.
//

#ifndef CIRCUITVIEW_H
#define CIRCUITVIEW_H
#include <unordered_map>
#include <vector>

#include "SFML/System/Vector2.hpp"


class CircuitView {
private:
  std::unordered_map<unsigned int, sf::Vector2f> nodePositions;
public:
  CircuitView(){}
  ~CircuitView() {}


  std::vector<std::pair<unsigned int, sf::Vector2f>> getNodes() const {
    std::vector<std::pair<unsigned int, sf::Vector2f>> nodes;
    for (auto& node: nodePositions) {
      nodes.emplace_back(node.first, node.second);
    }
    return nodes;
  }

  void recordNode(unsigned int nodeId, sf::Vector2f position) {
    nodePositions[nodeId] = position;
  }
};



#endif //CIRCUITVIEW_H
