#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class SnakeRenderer {
public:
    SnakeRenderer();

    bool loadSprites();
    bool isLoaded() const { return loaded; }
    void setSpriteScale(float scale) { spriteScale = scale; }
    float getSpriteScale() const { return spriteScale; }
    void setTailRotate180(bool v) { tailRotate180 = v; }
    bool getTailRotate180() const { return tailRotate180; }

    void drawHead(sf::RenderWindow& window, int x, int y, int blockSize, int dirX, int dirY);
    void drawBody(sf::RenderWindow& window, int x, int y, int blockSize, int dirX, int dirY);
    void drawTail(sf::RenderWindow& window, int x, int y, int blockSize, int dirX, int dirY);

private:
    sf::Texture headTexture, bodyTexture, tailTexture;
    bool loaded = false;

    bool loadTexture(sf::Texture& tex, const std::string& path);
    float spriteScale = 1.5f; // Multiply sprite rendering size relative to blockSize
    bool tailRotate180 = true;
};
