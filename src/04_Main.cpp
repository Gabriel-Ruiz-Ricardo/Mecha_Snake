#include <SFML/Graphics.hpp>
#include "GameLogic.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

const int BLOCKS = 60;
const int BLOCK_SIZE = 32;

int main() {
    // Get desktop resolution and compute window size that fits the game grid
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    const int marginWidth = 50; // leave some space at sides
    const int marginHeight = 100; // leave space for taskbar and OS chrome

    int maxAllowedWidth = std::max(800, (int)desktopMode.width - marginWidth);
    int maxAllowedHeight = std::max(600, (int)desktopMode.height - marginHeight);

    // Desired pixel size of grid
    int desiredWidth = BLOCKS * BLOCK_SIZE;
    int desiredHeight = BLOCKS * BLOCK_SIZE;

    // If desired size is larger than available desktop size, scale block size down
    int usedBlockSize = BLOCK_SIZE;
    if (desiredWidth > maxAllowedWidth || desiredHeight > maxAllowedHeight) {
        float scaleX = (float)maxAllowedWidth / (float)desiredWidth;
        float scaleY = (float)maxAllowedHeight / (float)desiredHeight;
        float scale = std::min(scaleX, scaleY);
        usedBlockSize = std::max(8, (int)std::floor(BLOCK_SIZE * scale));
    }

    int windowWidth = BLOCKS * usedBlockSize;
    int windowHeight = BLOCKS * usedBlockSize;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Snake - Classic", sf::Style::Default);
    window.setFramerateLimit(60);
    std::cout << "Window: " << windowWidth << "x" << windowHeight << ", blockSize: " << usedBlockSize << std::endl;
    
    GameLogic game(BLOCKS, BLOCKS, usedBlockSize);
    
    sf::Clock clock;
    float moveTimer = 0.0f;
    const float MOVE_INTERVAL = 0.08f; // Tiempo entre movimientos
    
    std::cout << "=== SNAKE GAME ===" << std::endl;
    std::cout << "Controls: Arrow keys or WASD to move" << std::endl;
    std::cout << "Press R to reset after Game Over" << std::endl;
    std::cout << "Press Enter to start from Menu" << std::endl;
    std::cout << "Press P to pause/resume" << std::endl;
    std::cout << "Avoid white walls and don't hit yourself" << std::endl;
    std::cout << "Press [ / ] to change sprite scale" << std::endl;
    std::cout << "==================" << std::endl;
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::R) {
                    game.reset();
                }
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
                if (event.key.code == sf::Keyboard::P) {
                    game.togglePause();
                }
                if (event.key.code == sf::Keyboard::T) {
                    game.toggleTailRotate();
                }
                if (event.key.code == sf::Keyboard::Enter) {
                    if (game.isMenu()) {
                        game.startGame();
                    } else if (game.isGameOver()) {
                        game.reset();
                        game.startGame();
                    }
                }
            }
        }
        
        // Manejo de entrada continuo
        game.handleInput();
        
        // Actualizar lógica del juego
        sf::Time elapsed = clock.restart();
        moveTimer += elapsed.asSeconds();
        
        if (moveTimer >= MOVE_INTERVAL) {
            moveTimer = 0.0f;
            game.update();
        }
        
        // Renderizar
        window.clear(sf::Color::Black);
        game.draw(window);
        window.display();
    }
    
    return 0;
}
