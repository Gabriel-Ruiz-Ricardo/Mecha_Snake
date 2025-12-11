#include <SFML/Graphics.hpp>
#include "GameLogic.hpp"
#include <iostream>

const int BLOCKS = 60;
const int BLOCK_SIZE = 32;

int main() {
    // Get desktop resolution and create maximized window
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    int windowWidth = desktopMode.width;
    int windowHeight = desktopMode.height - 40; // Leave space for taskbar
    
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Snake - Classic", sf::Style::Default);
    window.setFramerateLimit(60);
    
    GameLogic game(BLOCKS, BLOCKS, BLOCK_SIZE);
    
    sf::Clock clock;
    float moveTimer = 0.0f;
    const float MOVE_INTERVAL = 0.08f; // Tiempo entre movimientos
    
    std::cout << "=== SNAKE GAME ===" << std::endl;
    std::cout << "Controles: Flechas o WASD para mover" << std::endl;
    std::cout << "Presiona R para reiniciar después de Game Over" << std::endl;
    std::cout << "Evita los muros blancos y no te choques contigo misma" << std::endl;
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
