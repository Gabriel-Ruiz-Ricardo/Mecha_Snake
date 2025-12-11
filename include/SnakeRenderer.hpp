#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class SnakeRenderer {
public:
    SnakeRenderer();

    bool loadSprites();
    bool isLoaded() const { return loaded; }

    void drawHead(sf::RenderWindow& window, int x, int y, int blockSize, int dirX, int dirY);
    void drawBody(sf::RenderWindow& window, int x, int y, int blockSize, int dirX, int dirY);
    void drawTail(sf::RenderWindow& window, int x, int y, int blockSize, int dirX, int dirY);

private:
    sf::Texture headTexture, bodyTexture, tailTexture;
    bool loaded = false;

    bool loadTexture(sf::Texture& tex, const std::string& path);
};
