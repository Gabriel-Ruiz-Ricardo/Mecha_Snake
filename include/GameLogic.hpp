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
    void setSpriteScale(float s);
    void togglePause();
    void startGame();
    void toggleTailRotate();
    bool isPaused() const { return state == State::Paused; }
    bool isMenu() const { return state == State::Menu; }
    
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
    // UI
    sf::Font uiFont;
    sf::Font titleFont; // second font for title/pause
    sf::Text scoreText;
    sf::Text timerText;
    sf::Clock startClock;
    float elapsedPlaySeconds = 0.f; // time excluding paused
    float pausedAccumSeconds = 0.f;
    float finalElapsedSeconds = 0.f; // store elapsed seconds when game over
    sf::Clock pauseClock;
    bool paused = false;
    enum class State { Menu, Playing, Paused, GameOver };
    State state = State::Menu;
    float spriteScale = 2.0f;
};
