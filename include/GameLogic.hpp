#pragma once

#include <SFML/Graphics.hpp>
#include "Snake.hpp"
#include "Barrier.hpp"
#include "SnakeRenderer.hpp"
#include <random>
#include <SFML/Audio.hpp>

struct Food {
    int x;
    int y;
    
    bool operator==(const Cell &c) const {
        return x == c.x && y == c.y;
    }
};

class GameLogic {
public:
    GameLogic(int gridWidth, int gridHeight, int blockSize);
    
    void update();
    void handleInput();
    void draw(sf::RenderWindow& window);
    
    bool isGameOver() const { return gameOver; }
    int getScore() const { return score; }
    
    void reset();
    
private:
    Snake snake;
    Barrier barriers;
    Food food;
    SnakeRenderer renderer;
    sf::Music music;
    
    int gridWidth;
    int gridHeight;
    int blockSize;
    int score;
    bool gameOver;
    
    std::mt19937 rng;
    
    void spawnFood();
};
