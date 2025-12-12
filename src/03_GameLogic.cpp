#include "GameLogic.hpp"
#include <iostream>
#include <ctime>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>

GameLogic::GameLogic(int gridWidth, int gridHeight, int blockSize)
    : gridWidth(gridWidth), gridHeight(gridHeight), blockSize(blockSize),
      snake(gridWidth/2, gridHeight/2, sf::Color::Green),
      barriers(2, 2, gridWidth-3, gridHeight-3),
      score(0), gameOver(false) {
    rng.seed((unsigned)time(nullptr));

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
    // Load fruit textures and high score
    loadFruitTextures();
    loadHighScore();
}

void GameLogic::spawnFood() {
    // Spawn a common Gomu fruit at a valid position
    std::uniform_int_distribution<int> distX(barriers.getMinX() + 1, barriers.getMaxX() - 1);
    std::uniform_int_distribution<int> distY(barriers.getMinY() + 1, barriers.getMaxY() - 1);

    Fruit f;
    bool valid = false;
    int attempts = 0;
    while (!valid && attempts < 100) {
        f.x = distX(rng);
        f.y = distY(rng);
        valid = true;
        for (const auto &cell : snake.getBody()) {
            if (cell.x == f.x && cell.y == f.y) { valid = false; break; }
        }
        for (const auto &of : fruits) {
            if (of.x == f.x && of.y == f.y) { valid = false; break; }
        }
        attempts++;
    }
    if (!valid) return;
    f.type = Fruit::Type::Gomu;
    f.spawnTime = startClock.getElapsedTime().asSeconds() - pausedAccumSeconds;
    f.duration = 0.f;
    fruits.push_back(f);
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
        if (score > highScore) {
            awaitingNameEntry = true;
            nameBuffer.clear();
        } else {
            awaitingNameEntry = false;
        }
        return;
    }
    
    // Comprobar colisión consigo misma
    if (snake.checkSelfCollision()) {
        gameOver = true;
        state = State::GameOver;
        finalElapsedSeconds = startClock.getElapsedTime().asSeconds() - pausedAccumSeconds;
        std::cout << "Game Over! You collided with yourself. Score: " << score << std::endl;
        if (score > highScore) {
            awaitingNameEntry = true;
            nameBuffer.clear();
        } else {
            awaitingNameEntry = false;
        }
        return;
    }
    
    // Check fruits eaten
    for (size_t i = 0; i < fruits.size(); ++i) {
        if (head.x == fruits[i].x && head.y == fruits[i].y) {
            // handle eating by type
            switch (fruits[i].type) {
                case Fruit::Type::Gomu:
                    score += 1;
                    snake.grow();
                    // classic: when gomu eaten, spawn another gomu elsewhere
                    fruits.erase(fruits.begin() + (int)i);
                    spawnFood();
                    break;
                case Fruit::Type::Mera:
                    score += 5;
                    // grow 2 segments
                    snake.grow();
                    snake.grow();
                    fruits.erase(fruits.begin() + (int)i);
                    break;
                case Fruit::Type::Ope:
                    score += 10;
                    // grow 3 segments
                    snake.grow();
                    snake.grow();
                    snake.grow();
                    fruits.erase(fruits.begin() + (int)i);
                    break;
            }
            break;
        }
    }

    // spawn checks every interval
    float now = elapsedPlaySeconds;
    if (now - lastSpawnCheck >= spawnCheckInterval) {
        spawnCheck(now);
        lastSpawnCheck = now;
    }

    // remove expired temporary fruits
    removeExpired(now);
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
        // Show highest score and name using second font if available
        if (titleFont.getInfo().family.size()) {
            std::string scoreStr = std::to_string(highScore);
            sf::Text hs(std::string("Highest score: ") + scoreStr + std::string(" - \"") + highName + std::string("\""), titleFont, std::max(16, blockSize / 2));
            hs.setFillColor(sf::Color::White);
            sf::FloatRect hb = hs.getLocalBounds();
            hs.setOrigin(hb.width / 2.f, hb.height / 2.f);
            hs.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f - 48.f);
            window.draw(hs);
        }
        // Show Ctrl+R reset hint in lower right corner (larger)
        if (titleFont.getInfo().family.size()) {
            sf::Text hint("Ctrl + R to erase all data", titleFont, std::max(14, blockSize / 2));
            hint.setFillColor(sf::Color::White);
            sf::FloatRect hintBounds = hint.getLocalBounds();
            hint.setPosition((float)(gridWidth * blockSize) - hintBounds.width - (float)blockSize * 0.5f, (float)(gridHeight * blockSize) - hintBounds.height - (float)blockSize * 0.5f);
            window.draw(hint);
        }
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

        // Draw all fruits using textures
        for (const auto &f : fruits) {
            sf::Sprite s;
            if (f.type == Fruit::Type::Gomu) s.setTexture(texGomu);
            else if (f.type == Fruit::Type::Mera) s.setTexture(texMera);
            else s.setTexture(texOpe);
            sf::Vector2u ts = s.getTexture()->getSize();
            float texW = (float)ts.x;
            float texH = (float)ts.y;
            float sizeInPixels = (float)blockSize * renderer.getSpriteScale();
            float scaleX = sizeInPixels / texW;
            float scaleY = sizeInPixels / texH;
            s.setScale(scaleX, scaleY);
            s.setOrigin(texW/2.f, texH/2.f);
            s.setPosition((float)(f.x * blockSize) + (float)blockSize/2.f, (float)(f.y * blockSize) + (float)blockSize/2.f);
            window.draw(s);
        }
    } else {
        // Fallback: dibujar rectángulos sólidos
        snake.draw(window, blockSize);
        for (const auto &f : fruits) {
            float sizeInPixels = (float)blockSize * renderer.getSpriteScale();
            sf::RectangleShape fs({sizeInPixels, sizeInPixels});
            if (f.type == Fruit::Type::Gomu) fs.setFillColor(sf::Color(180,200,255));
            else if (f.type == Fruit::Type::Mera) fs.setFillColor(sf::Color(255,150,50));
            else fs.setFillColor(sf::Color(255,220,100));
            float off = ((float)blockSize - sizeInPixels) / 2.f;
            fs.setPosition((float)(f.x * blockSize) + off, (float)(f.y * blockSize) + off);
            window.draw(fs);
        }
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
                lt.setPosition(x + bb.width / 2.f, (float)(gridHeight * blockSize) / 4.f + yoff);
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
        finalScore.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f - 40.f);
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
        finalTime.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f);
        window.draw(finalTime);

        // High score info (only show if not entering name)
        if (!awaitingNameEntry) {
            std::string hsStr = std::string("Best Score: ") + std::to_string(highScore) + std::string(" - \"") + highName + std::string("\"");
            sf::Text highScoreText(hsStr, uiFont, scoreText.getCharacterSize());
            highScoreText.setFillColor(sf::Color::Yellow);
            sf::FloatRect hsb = highScoreText.getLocalBounds();
            highScoreText.setOrigin(hsb.width / 2.f, hsb.height / 2.f);
            highScoreText.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f + 40.f);
            window.draw(highScoreText);
        }

        // Restart prompt or name entry if new record
        if (awaitingNameEntry && titleFont.getInfo().family.size()) {
            sf::Text prompt("You broke the record! Enter your name:", titleFont, std::max(18, blockSize/1));
            prompt.setFillColor(sf::Color::White);
            sf::FloatRect pb = prompt.getLocalBounds();
            prompt.setOrigin(pb.width / 2.f, pb.height / 2.f);
            prompt.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f + 50.f);
            window.draw(prompt);

            // show current typed name
            sf::Text nameText(nameBuffer.empty() ? std::string("_") : nameBuffer, titleFont, std::max(18, blockSize/1));
            nameText.setFillColor(sf::Color::White);
            sf::FloatRect nb = nameText.getLocalBounds();
            nameText.setOrigin(nb.width / 2.f, nb.height / 2.f);
            nameText.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f + 100.f);
            window.draw(nameText);
        } else {
            sf::Text prompt("Press Enter to Restart", uiFont, std::max(18, blockSize));
            prompt.setFillColor(sf::Color::White);
            sf::FloatRect pb = prompt.getLocalBounds();
            prompt.setOrigin(pb.width / 2.f, pb.height / 2.f);
            prompt.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f + 80.f);
            window.draw(prompt);
        }
        // Show Ctrl+R reset hint in lower right corner even on game over (larger)
        if (titleFont.getInfo().family.size()) {
            sf::Text hint("Ctrl + R to erase all data", titleFont, std::max(14, blockSize / 2));
            hint.setFillColor(sf::Color::White);
            sf::FloatRect hintBounds = hint.getLocalBounds();
            hint.setPosition((float)(gridWidth * blockSize) - hintBounds.width - (float)blockSize * 0.5f, (float)(gridHeight * blockSize) - hintBounds.height - (float)blockSize * 0.5f);
            window.draw(hint);
        }
    }
}

void GameLogic::reset() {
    snake.reset(gridWidth / 2, gridHeight / 2);
    score = 0;
    gameOver = false;
    lastSpawnCheck = 0.f;
    fruits.clear();
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
    lastSpawnCheck = 0.f;
    snake.reset(gridWidth / 2, gridHeight / 2);
    fruits.clear();
    spawnFood();
    score = 0;
    gameOver = false;
    finalElapsedSeconds = 0.f;
}

void GameLogic::toggleTailRotate() {
    renderer.setTailRotate180(!renderer.getTailRotate180());
}

void GameLogic::loadFruitTextures() {
    if (!texGomu.loadFromFile("../assets/images/gomu_gomu.png")) texGomu.loadFromFile("assets/images/gomu_gomu.png");
    if (!texMera.loadFromFile("../assets/images/mera_mera.png")) texMera.loadFromFile("assets/images/mera_mera.png");
    if (!texOpe.loadFromFile("../assets/images/ope_ope.png")) texOpe.loadFromFile("assets/images/ope_ope.png");
}

void GameLogic::spawnCheck(float nowSeconds) {
    std::uniform_real_distribution<float> dist01(0.f, 1.f);
    const float pMera = 0.18f;
    const float pOpe  = 0.05f;

    auto trySpawn = [&](Fruit::Type t, float duration){
        // don't spawn this type if one already exists
        for (const auto &of : fruits) if (of.type == t) return;

        std::uniform_int_distribution<int> distX(barriers.getMinX() + 1, barriers.getMaxX() - 1);
        std::uniform_int_distribution<int> distY(barriers.getMinY() + 1, barriers.getMaxY() - 1);
        Fruit f;
        bool valid = false;
        int attempts = 0;
        while (!valid && attempts < 30) {
            f.x = distX(rng);
            f.y = distY(rng);
            valid = true;
            for (const auto &cell : snake.getBody()) {
                if (cell.x == f.x && cell.y == f.y) { valid = false; break; }
            }
            for (const auto &of : fruits) {
                if (of.x == f.x && of.y == f.y) { valid = false; break; }
            }
            attempts++;
        }
        if (valid) {
            f.type = t;
            f.spawnTime = nowSeconds;
            f.duration = duration;
            fruits.push_back(f);
        }
    };

    // Gomu is the persistent single food handled by spawnFood()/eating.
    // Sporadic single-instance fruits: Mera and Ope
    if (dist01(rng) < pMera) trySpawn(Fruit::Type::Mera, 4.f);
    if (dist01(rng) < pOpe)  trySpawn(Fruit::Type::Ope, 2.f);
}

void GameLogic::removeExpired(float nowSeconds) {
    fruits.erase(std::remove_if(fruits.begin(), fruits.end(), [&](const Fruit &f){
        if (f.duration <= 0.f) return false;
        return (nowSeconds - f.spawnTime) >= f.duration;
    }), fruits.end());
}

void GameLogic::processEvent(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        // Ctrl+R to reset stats (ignore during name entry)
        if (!awaitingNameEntry && event.key.code == sf::Keyboard::R && event.key.control) {
            highScore = 0;
            highName = "Nobody";
            saveHighScore();
            return;
        }
    }

    if (!awaitingNameEntry) return;
    if (event.type == sf::Event::TextEntered) {
        uint32_t ch = event.text.unicode;
        if (ch == 8) { // backspace
            if (!nameBuffer.empty()) nameBuffer.pop_back();
        } else if (ch >= 32 && ch < 127) { // printable ASCII including space
            nameBuffer.push_back((char)ch);
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Enter) {
            // finalize name
            if (!nameBuffer.empty()) {
                highScore = score;
                highName = nameBuffer;
                saveHighScore();
            }
            awaitingNameEntry = false;
        }
    }
}

void GameLogic::loadHighScore() {
    std::vector<std::string> candidates = {"../assets/highscore.txt", "assets/highscore.txt", "highscore.txt"};
    for (const auto &p : candidates) {
        std::ifstream in(p);
        if (!in) continue;
        int s = 0;
        std::string name;
        if (in >> s) {
            std::getline(in, name); // consume rest of line
            std::string line;
            if (std::getline(in, line)) {
                // second line could be name
                name = line;
            }
            highScore = s;
            if (!name.empty()) {
                // trim leading spaces
                size_t pos = name.find_first_not_of(" \t\r\n");
                if (pos != std::string::npos) name = name.substr(pos);
                highName = name;
            }
            return;
        }
    }
    highScore = 0;
    highName = "Nobody";
}

void GameLogic::saveHighScore() {
    std::ofstream out("highscore.txt");
    if (!out) return;
    out << highScore << "\n";
    out << highName << "\n";
}
