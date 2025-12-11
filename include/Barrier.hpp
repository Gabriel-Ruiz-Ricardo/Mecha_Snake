#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include "Common.hpp"

class Barrier {
public:
    Barrier(int minX, int minY, int maxX, int maxY);
    
    void draw(sf::RenderWindow& window, int blockSize);
    bool checkCollision(const Cell& pos) const;
    
    int getMinX() const { return minX; }
    int getMinY() const { return minY; }
    int getMaxX() const { return maxX; }
    int getMaxY() const { return maxY; }
    
private:
    int minX, minY, maxX, maxY;
    std::vector<Cell> walls;
    
    void buildWalls();
};
