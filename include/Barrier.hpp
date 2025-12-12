#pragma once

#include <vector>
#include <random>
#include <SFML/Graphics.hpp>
#include "Common.hpp"

class Barrier {
public:
    Barrier(int minX, int minY, int maxX, int maxY);
    
    void draw(sf::RenderWindow& window, int blockSize);
    bool checkCollision(const Cell& pos) const;
    // regenerate random internal walls while keeping border
    void generateRandom(std::mt19937 &rng, int gridWidth, int gridHeight, const std::vector<Cell>& forbidden);
    const std::vector<Cell>& getWalls() const { return walls; }
    void loadTexture(const std::string& path);
    
    int getMinX() const { return minX; }
    int getMinY() const { return minY; }
    int getMaxX() const { return maxX; }
    int getMaxY() const { return maxY; }
    
private:
    int minX, minY, maxX, maxY;
    std::vector<Cell> walls;
    sf::Texture wallTexture;
    
    void buildWalls();
};
