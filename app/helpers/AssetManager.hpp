//
// Created by Nazarii on 11/1/25.
//

#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H
#include <iostream>
#include <string>
#include <unordered_map>

#include "SFML/Graphics/Texture.hpp"


class AssetManager {
private:
    std::unordered_map<std::string, sf::Texture> textures;
    std::string root;
public:
    AssetManager(const std::string& root) : root(root), textures() {
        loadTexture("capacitor", "capacitor.png");
        loadTexture("resistor", "resistor.png");
        loadTexture("isource", "isource.png");
        loadTexture("vsource", "vsource.png");
        loadTexture("wire", "wire.png");
        loadTexture("edit", "edit.png");
        loadTexture("play", "play.png");
        loadTexture("pause", "pause.png");
        loadTexture("gear", "gear.png");
    }
    ~AssetManager() {}

    sf::Texture& getTexture(const std::string& name) {
        return textures.at(name);
    }

    sf::Texture loadTexture(const std::string& name, const std::string& relPath) {
        if (textures.contains(name)) {
            throw std::runtime_error("Texture already exists");
        }

        sf::Texture texture;

        if (!texture.loadFromFile(root + relPath))
            throw std::runtime_error("Could not load " + root + relPath);

        textures.emplace(name, texture);

        return texture;
    }
};



#endif //ASSETMANAGER_H
