#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include "Common.hpp"

class Snake {
public:
    Snake(int startX, int startY, sf::Color color);
    
    void changeDirection(int dx, int dy);
    void update();
    void draw(sf::RenderWindow& window, int blockSize);
    
    bool checkSelfCollision() const;
    Cell getHead() const { return body.front(); }
    const std::vector<Cell>& getBody() const { return body; }
    Cell getDirection() const { return direction; }
    
    void grow();
    void reset(int startX, int startY);
    void setBody(const std::vector<Cell>& b);
    void shrinkTo(int len);
    
private:
    std::vector<Cell> body;
    Cell direction;
    Cell nextDirection;
    sf::Color color;
};
