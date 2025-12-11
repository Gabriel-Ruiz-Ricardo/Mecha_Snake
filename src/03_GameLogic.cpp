#include "GameLogic.hpp"
#include <iostream>
#include <ctime>

GameLogic::GameLogic(int gridWidth, int gridHeight, int blockSize)
    : gridWidth(gridWidth), gridHeight(gridHeight), blockSize(blockSize),
      snake(gridWidth/2, gridHeight/2, sf::Color::Green),
      barriers(2, 2, gridWidth-3, gridHeight-3),
      score(0), gameOver(false) {
    rng.seed((unsigned)time(nullptr));
    spawnFood();

    // Load weedle sprites (head, body_1-4, tail)
    if (renderer.loadSprites()) {
        std::cout << "Weedle sprites loaded successfully\n";
    } else {
        std::cout << "Failed to load some weedle sprites\n";
    }

    // Load background music if present
    if (music.openFromFile("../assets/music/snow_city.mp3")) {
        music.setLoop(true);
        music.setVolume(50.f);
        music.play();
        std::cout << "Background music loaded and playing\n";
    } else {
        std::cout << "Background music not found or failed to load\n";
    }
}

void GameLogic::spawnFood() {
    std::uniform_int_distribution<int> distX(barriers.getMinX() + 1, barriers.getMaxX() - 1);
    std::uniform_int_distribution<int> distY(barriers.getMinY() + 1, barriers.getMaxY() - 1);
    
    Food newFood;
    bool validPosition = false;
    
    while (!validPosition) {
        newFood.x = distX(rng);
        newFood.y = distY(rng);
        
        // Verificar que no esté en la serpiente
        validPosition = true;
        for (const auto& cell : snake.getBody()) {
            if (cell.x == newFood.x && cell.y == newFood.y) {
                validPosition = false;
                break;
            }
        }
    }
    
    food = newFood;
}

void GameLogic::handleInput() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        snake.changeDirection(0, -1);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        snake.changeDirection(0, 1);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        snake.changeDirection(-1, 0);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        snake.changeDirection(1, 0);
    }
}

void GameLogic::update() {
    if (gameOver) return;
    
    snake.update();
    Cell head = snake.getHead();
    
    // Comprobar colisión con barreras
    if (barriers.checkCollision(head)) {
        gameOver = true;
        std::cout << "¡Game Over! Chocaste con una barrera. Puntuación: " << score << std::endl;
        return;
    }
    
    // Comprobar colisión consigo misma
    if (snake.checkSelfCollision()) {
        gameOver = true;
        std::cout << "¡Game Over! Te chocaste contigo misma. Puntuación: " << score << std::endl;
        return;
    }
    
    // Comprobar si comió comida
    if (head.x == food.x && head.y == food.y) {
        snake.grow();
        score += 10;
        spawnFood();
    }
}

void GameLogic::draw(sf::RenderWindow& window) {
    // Dibujar barreras
    barriers.draw(window, blockSize);

    // Dibujar serpiente con sprites Weedle
    const auto& bodyCells = snake.getBody();
    Cell dir = snake.getDirection();
    
    if (renderer.isLoaded() && !bodyCells.empty()) {
        // head (con rotación según dirección)
        renderer.drawHead(window, bodyCells[0].x, bodyCells[0].y, blockSize, dir.x, dir.y);

        // body segments (1 to n-1) y tail (último segmento)
        for (size_t i = 1; i < bodyCells.size(); ++i) {
            if (i == bodyCells.size() - 1) {
                // Último segmento = tail (con rotación)
                renderer.drawTail(window, bodyCells[i].x, bodyCells[i].y, blockSize, dir.x, dir.y);
            } else {
                // Body segments: compute local orientation using previous and next cell
                const Cell& prev = bodyCells[i - 1];
                const Cell& curr = bodyCells[i];
                const Cell& next = bodyCells[i + 1];

                int dx = next.x - prev.x;
                int dy = next.y - prev.y;

                int segDirX = 0;
                int segDirY = -1; // default up

                if (dx != 0) {
                    segDirX = (dx > 0) ? 1 : -1;
                    segDirY = 0;
                } else if (dy != 0) {
                    segDirX = 0;
                    segDirY = (dy > 0) ? 1 : -1;
                }

                renderer.drawBody(window, bodyCells[i].x, bodyCells[i].y, blockSize, segDirX, segDirY);
            }
        }

        // Comida (rojo simple por ahora)
        sf::RectangleShape foodShape({(float)blockSize, (float)blockSize});
        foodShape.setFillColor(sf::Color::Red);
        foodShape.setPosition((float)(food.x * blockSize), (float)(food.y * blockSize));
        window.draw(foodShape);
    } else {
        // Fallback: dibujar rectángulos sólidos
        snake.draw(window, blockSize);
        sf::RectangleShape foodShape({(float)blockSize, (float)blockSize});
        foodShape.setFillColor(sf::Color::Red);
        foodShape.setPosition((float)(food.x * blockSize), (float)(food.y * blockSize));
        window.draw(foodShape);
    }
}

void GameLogic::reset() {
    snake.reset(gridWidth / 2, gridHeight / 2);
    score = 0;
    gameOver = false;
    spawnFood();
}
