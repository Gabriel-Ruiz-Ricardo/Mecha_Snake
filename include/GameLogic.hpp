#pragma once

#include <SFML/Graphics.hpp>
#include "Snake.hpp"
#include "Barrier.hpp"
#include "SnakeRenderer.hpp"
#include <random>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>

struct Fruit {
    enum class Type { Gomu, Mera, Ope } type;
    int x;
    int y;
    float spawnTime; // seconds since game start
    float duration; // 0 = permanent until eaten

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
    // event processing for text input (high-score name entry)
    void processEvent(const sf::Event& event);
    
    bool isGameOver() const { return gameOver; }
    int getScore() const { return score; }
    
    void reset();
    void setSpriteScale(float s);
    void togglePause();
    void startGame();
    void toggleTailRotate();
    bool canRestart() const { return !awaitingNameEntry; }
    bool isPaused() const { return state == State::Paused; }
    bool isMenu() const { return state == State::Menu; }
    
private:
    Snake snake;
    Barrier barriers;
    // support multiple fruits on the board
    std::vector<Fruit> fruits;
    SnakeRenderer renderer;
    sf::Music music;
    
    int gridWidth;
    int gridHeight;
    int blockSize;
    int score;
    bool gameOver;
    
    std::mt19937 rng;

    void spawnFood();
    void spawnCheck(float nowSeconds);
    void removeExpired(float nowSeconds);
    void loadFruitTextures();

    

    // high score persistence
    void loadHighScore();
    void saveHighScore();
    int highScore = 0;
    std::string highName = "Nobody";
    std::string nameBuffer; // temporary buffer when entering name
    float lastSpawnCheck = 0.f;
    float spawnCheckInterval = 0.5f; // seconds

    bool awaitingNameEntry = false;

    // fruit textures
    sf::Texture texGomu;
    sf::Texture texMera;
    sf::Texture texOpe;
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
