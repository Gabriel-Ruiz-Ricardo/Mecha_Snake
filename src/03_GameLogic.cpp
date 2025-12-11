#include "GameLogic.hpp"
#include <iostream>
#include <ctime>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <vector>

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
    // Set sprite scale for renderer
    renderer.setSpriteScale(spriteScale);

    // Load UI font (try Minecraft first then fallback) and setup texts
    // Try preferred path; if it fails, try without the ".." prefix
    bool fontLoaded = false;
    if (uiFont.loadFromFile("../assets/fonts/Minecraft.ttf")) {
        std::cout << "Loaded UI font: ../assets/fonts/Minecraft.ttf\n";
        fontLoaded = true;
    } else if (uiFont.loadFromFile("assets/fonts/Minecraft.ttf")) {
        std::cout << "Loaded UI font: assets/fonts/Minecraft.ttf\n";
        fontLoaded = true;
    } else if (uiFont.loadFromFile("../assets/fonts/HOMOARAK.TTF")) {
        std::cout << "Loaded UI font: ../assets/fonts/HOMOARAK.TTF\n";
        fontLoaded = true;
    } else if (uiFont.loadFromFile("assets/fonts/HOMOARAK.TTF")) {
        std::cout << "Loaded UI font: assets/fonts/HOMOARAK.TTF\n";
        fontLoaded = true;
    } else {
        std::cerr << "Failed to load UI font from assets/fonts\n";
    }
    scoreText.setFont(uiFont);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setOutlineColor(sf::Color::Black);
    scoreText.setOutlineThickness(2.f);
    scoreText.setCharacterSize(std::max(24, (int)(blockSize * 1.25f)));
    scoreText.setStyle(sf::Text::Bold);

    timerText.setFont(uiFont);
    timerText.setFillColor(sf::Color::White);
    timerText.setOutlineColor(sf::Color::Black);
    timerText.setOutlineThickness(2.f);
    timerText.setCharacterSize(std::max(24, (int)(blockSize * 1.25f)));
    timerText.setStyle(sf::Text::Bold);

    startClock.restart();
    pausedAccumSeconds = 0.f;
    paused = false;
    state = State::Menu;

    // Load title font (HOMOARAK)
    if (titleFont.loadFromFile("../assets/fonts/HOMOARAK.TTF")) {
        std::cout << "Loaded title font: ../assets/fonts/HOMOARAK.TTF\n";
    } else if (titleFont.loadFromFile("assets/fonts/HOMOARAK.TTF")) {
        std::cout << "Loaded title font: assets/fonts/HOMOARAK.TTF\n";
    } else {
        std::cerr << "Failed to load title font (HOMOARAK)\n";
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
    // Only process movement input while playing
    if (state == State::Playing) {
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

    // Allow adjusting sprite scale with [ and ] keys at any state
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LBracket)) {
        spriteScale = std::max(0.5f, spriteScale - 0.02f);
        renderer.setSpriteScale(spriteScale);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::RBracket)) {
        spriteScale = std::min(3.0f, spriteScale + 0.02f);
        renderer.setSpriteScale(spriteScale);
    }
}

void GameLogic::update() {
    if (gameOver) return;
    if (state == State::Menu) return;
    if (state == State::Paused) return;

    // accumulate play time using startClock minus pausedAccumSeconds
    elapsedPlaySeconds = startClock.getElapsedTime().asSeconds() - pausedAccumSeconds;

    snake.update();
    Cell head = snake.getHead();
    
    // Comprobar colisión con barreras
    if (barriers.checkCollision(head)) {
        gameOver = true;
        state = State::GameOver;
        finalElapsedSeconds = startClock.getElapsedTime().asSeconds() - pausedAccumSeconds;
        std::cout << "Game Over! You hit a wall. Score: " << score << std::endl;
        return;
    }
    
    // Comprobar colisión consigo misma
    if (snake.checkSelfCollision()) {
        gameOver = true;
        state = State::GameOver;
        finalElapsedSeconds = startClock.getElapsedTime().asSeconds() - pausedAccumSeconds;
        std::cout << "Game Over! You collided with yourself. Score: " << score << std::endl;
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

    // If in menu, draw title and prompt and return
    if (state == State::Menu) {
        if (titleFont.getInfo().family.size()) {
            std::string s = "SNAKE";
            int fontSize = std::max(64, blockSize * 2);
            float totalWidth = 0.f;
            std::vector<sf::Text> letters;
            for (char c : s) {
                sf::Text t( std::string(1, c), titleFont, fontSize);
                sf::FloatRect bb = t.getLocalBounds();
                totalWidth += bb.width;
                letters.push_back(t);
            }
            float startX = ((float)gridWidth * blockSize - totalWidth) / 2.f;
            float ttime = startClock.getElapsedTime().asSeconds();
            float x = startX;
            for (size_t i = 0; i < letters.size(); ++i) {
                sf::Text &lt = letters[i];
                lt.setFillColor(sf::Color::White);
                lt.setOutlineColor(sf::Color::Black);
                lt.setOutlineThickness(3.f);
                float amp = 12.f;
                float yoff = std::sin(ttime * 2.0f + (float)i * 0.7f) * amp;
                sf::FloatRect bb = lt.getLocalBounds();
                lt.setOrigin(bb.width / 2.f, bb.height / 2.f);
                lt.setPosition(x + bb.width / 2.f, (float)(gridHeight * blockSize) / 4.f + yoff);
                window.draw(lt);
                x += bb.width;
            }
        }
        sf::Text prompt("Press Enter to Play", titleFont.getInfo().family.size() ? titleFont : uiFont, std::max(18, blockSize));
        prompt.setFillColor(sf::Color::White);
        sf::FloatRect pb = prompt.getLocalBounds();
        prompt.setOrigin(pb.width / 2.f, pb.height / 2.f);
        prompt.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f);
        window.draw(prompt);
        return;
    }

    // Dibujar serpiente con sprites Weedle
    const auto& bodyCells = snake.getBody();
    Cell dir = snake.getDirection();
    
    if (renderer.isLoaded() && !bodyCells.empty()) {
        // Draw body segments and tail first, then draw head over them
        for (size_t i = 1; i < bodyCells.size(); ++i) {
                if (i == bodyCells.size() - 1) {
                // tail: orientation determined by vector from tail to previous cell
                const Cell& prev = bodyCells[i - 1];
                const Cell& curr = bodyCells[i];
                    int segDirX = curr.x - prev.x; // direction from tail to prev
                    int segDirY = curr.y - prev.y;
                renderer.drawTail(window, bodyCells[i].x, bodyCells[i].y, blockSize, segDirX, segDirY);
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

        // head drawn last to ensure it's on top
        renderer.drawHead(window, bodyCells[0].x, bodyCells[0].y, blockSize, dir.x, dir.y);

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

    // UI: score + timer (draw on top)
    scoreText.setString("Score: " + std::to_string(score));
    // Leave larger padding from the border for score display
    scoreText.setPosition((float)blockSize * 1.5f, 5.f);
    // Background for score
    {
        sf::FloatRect bounds = scoreText.getLocalBounds();
        sf::RectangleShape back({bounds.width + 8.f, bounds.height + 8.f});
        back.setFillColor(sf::Color(0, 0, 0, 140));
        back.setPosition(scoreText.getPosition().x - 4.f, scoreText.getPosition().y - 4.f);
        window.draw(back);
    }
    window.draw(scoreText);

    // compute elapsed shown to user excluding paused time
    float currentElapsed = startClock.getElapsedTime().asSeconds() - pausedAccumSeconds;
    float displayElapsed = currentElapsed;
    if (state == State::GameOver) displayElapsed = finalElapsedSeconds;
    int totalSeconds = (int)std::max(0.f, displayElapsed);
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);
    timerText.setString(std::string("Time: ") + buf);
    // Position on top-right
    float winW = (float)(gridWidth * blockSize);
    float textW = timerText.getLocalBounds().width;
    timerText.setPosition(winW - textW - (float)blockSize * 0.5f, 5.f);
    {
        sf::FloatRect bounds = timerText.getLocalBounds();
        sf::RectangleShape back({bounds.width + 8.f, bounds.height + 8.f});
        back.setFillColor(sf::Color(0, 0, 0, 140));
        back.setPosition(timerText.getPosition().x - 4.f, timerText.getPosition().y - 4.f);
        window.draw(back);
    }
    window.draw(timerText);

    // If paused, draw blinking PAUSE text in center using titleFont
    if (state == State::Paused) {
        if (titleFont.getInfo().family.size()) {
            float t = pauseClock.getElapsedTime().asSeconds();
            bool visible = (fmod(t, 1.0f) < 0.5f);
            if (visible) {
                sf::Text ptext("PAUSE", titleFont, std::max(48, blockSize * 2));
                ptext.setFillColor(sf::Color::White);
                ptext.setOutlineColor(sf::Color::Black);
                ptext.setOutlineThickness(3.f);
                sf::FloatRect b = ptext.getLocalBounds();
                ptext.setOrigin(b.width / 2.f, b.height / 2.f);
                ptext.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f);
                window.draw(ptext);
            }
        }
    }

    // If game over, show frozen overlay with final score and time
    if (state == State::GameOver) {
        // darken the scene slightly
        sf::RectangleShape overlay({(float)gridWidth * blockSize, (float)gridHeight * blockSize});
        overlay.setFillColor(sf::Color(0, 0, 0, 140));
        window.draw(overlay);

        // Big 'GAME OVER' title
        if (titleFont.getInfo().family.size()) {
            std::string s = "GAME OVER";
            int fontSize = std::max(48, blockSize * 2);
            float totalWidth = 0.f;
            std::vector<sf::Text> letters;
            for (char c : s) {
                sf::Text t( std::string(1, c), titleFont, fontSize);
                sf::FloatRect bb = t.getLocalBounds();
                totalWidth += bb.width;
                letters.push_back(t);
            }
            float startX = ((float)gridWidth * blockSize - totalWidth) / 2.f;
            float ttime = startClock.getElapsedTime().asSeconds();
            float x = startX;
            for (size_t i = 0; i < letters.size(); ++i) {
                sf::Text &lt = letters[i];
                lt.setFillColor(sf::Color::White);
                lt.setOutlineColor(sf::Color::Black);
                lt.setOutlineThickness(4.f);
                float amp = 12.f;
                float yoff = std::sin(ttime * 2.0f + (float)i * 0.7f) * amp;
                sf::FloatRect bb = lt.getLocalBounds();
                lt.setOrigin(bb.width / 2.f, bb.height / 2.f);
                lt.setPosition(x + bb.width / 2.f, (float)(gridHeight * blockSize) / 3.f + yoff);
                window.draw(lt);
                x += bb.width;
            }
        }

        // Score text in center
        sf::Text finalScore(scoreText.getString(), uiFont, scoreText.getCharacterSize());
        finalScore.setString(std::string("Score: ") + std::to_string(score));
        finalScore.setFillColor(sf::Color::White);
        sf::FloatRect sb = finalScore.getLocalBounds();
        finalScore.setOrigin(sb.width / 2.f, sb.height / 2.f);
        finalScore.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f - 20.f);
        window.draw(finalScore);

        // Time played (frozen at moment of GameOver)
        int totalSeconds = (int)finalElapsedSeconds;
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);
        sf::Text finalTime(timerText.getString(), uiFont, timerText.getCharacterSize());
        finalTime.setString(std::string("Time: ") + buf);
        finalTime.setFillColor(sf::Color::White);
        sf::FloatRect tb = finalTime.getLocalBounds();
        finalTime.setOrigin(tb.width / 2.f, tb.height / 2.f);
        finalTime.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f + 20.f);
        window.draw(finalTime);

        // Restart prompt
        sf::Text prompt("Press Enter to Restart", uiFont, std::max(18, blockSize));
        prompt.setFillColor(sf::Color::White);
        sf::FloatRect pb = prompt.getLocalBounds();
        prompt.setOrigin(pb.width / 2.f, pb.height / 2.f);
        prompt.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f + 60.f);
        window.draw(prompt);
    }
}

void GameLogic::reset() {
    snake.reset(gridWidth / 2, gridHeight / 2);
    score = 0;
    gameOver = false;
    spawnFood();
}

void GameLogic::setSpriteScale(float s) {
    spriteScale = s;
    renderer.setSpriteScale(spriteScale);
}

void GameLogic::togglePause() {
    if (state == State::Playing) {
        state = State::Paused;
        pauseClock.restart();
    } else if (state == State::Paused) {
        // Resume
        pausedAccumSeconds += pauseClock.getElapsedTime().asSeconds();
        state = State::Playing;
    }
}

void GameLogic::startGame() {
    state = State::Playing;
    startClock.restart();
    pausedAccumSeconds = 0.f;
    snake.reset(gridWidth / 2, gridHeight / 2);
    spawnFood();
    score = 0;
    gameOver = false;
    finalElapsedSeconds = 0.f;
}

void GameLogic::toggleTailRotate() {
    renderer.setTailRotate180(!renderer.getTailRotate180());
}
