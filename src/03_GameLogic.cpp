#include "GameLogic.hpp"
#include <iostream>
#include <ctime>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <random>

// ... rest of the file ...

void GameLogic::spawnFood() {
    // Do not spawn fruits if portal entrance is active
    if (portalEntrance.active) return;

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
        if (barriers.checkCollision({f.x, f.y})) valid = false;
        attempts++;
    }
    if (!valid) return;
    f.type = Fruit::Type::Gomu;
    f.spawnTime = startClock.getElapsedTime().asSeconds() - pausedAccumSeconds;
    f.duration = 0.f;
    fruits.push_back(f);
}
#include "GameLogic.hpp"
#include <iostream>
#include <ctime>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <fstream>

GameLogic::GameLogic(int gridWidth, int gridHeight, int blockSize)
    : gridWidth(gridWidth), gridHeight(gridHeight), blockSize(blockSize),
      snake(gridWidth/2, gridHeight/2, sf::Color::Green),
      barriers(2, 2, gridWidth-3, gridHeight-3),
      score(0), gameOver(false)
{
    // Definir lambda para buscar archivos en assets
    auto findAssetPath = [&](const std::string &p)->std::string {
        std::ifstream f(p);
        if (f.good()) { f.close(); return p; }
        if (p.rfind("../", 0) == 0) {
            std::string alt = p.substr(3);
            std::ifstream f2(alt);
            if (f2.good()) { f2.close(); return alt; }
        } else {
            std::string alt = std::string("../") + p;
            std::ifstream f2(alt);
            if (f2.good()) { f2.close(); return alt; }
        }
        return std::string();
    };

    // Cargar textura de portal
    std::string portalPath = findAssetPath("assets/images/portal.png");
    if (!portalPath.empty()) {
        if (portalTexture.loadFromFile(portalPath)) {
            std::cout << "Loaded portal texture: " << portalPath << "\n";
        } else {
            std::cerr << "Failed to load portal texture from: " << portalPath << "\n";
        }
    } else {
        std::cerr << "portal.png not found in candidates\n";
    }
    rng.seed((unsigned)time(nullptr));

    // Load weedle sprites (head, body_1-4, tail)
    if (renderer.loadSprites()) {
        std::cout << "Weedle sprites loaded successfully\n";
    } else {
        std::cout << "Failed to load some weedle sprites\n";
    }

    // Set sprite scale for renderer
    renderer.setSpriteScale(spriteScale);

    // Load wall texture for barriers
    barriers.loadTexture("assets/images/muro.jpeg");

    std::string fontPath = findAssetPath("assets/fonts/Minecraft.ttf");
    if (fontPath.empty()) fontPath = findAssetPath("assets/fonts/HOMOARAK.TTF");
    if (!fontPath.empty() && uiFont.loadFromFile(fontPath)) {
        std::cout << "Loaded UI font: " << fontPath << "\n";
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

    fruitTimerText.setFont(uiFont);
    fruitTimerText.setFillColor(sf::Color::White);
    fruitTimerText.setOutlineColor(sf::Color::Black);
    fruitTimerText.setOutlineThickness(2.f);
    fruitTimerText.setCharacterSize(std::max(20, (int)(blockSize * 1.0f)));
    fruitTimerText.setStyle(sf::Text::Bold);

    countdownText.setFont(uiFont);
    countdownText.setFillColor(sf::Color::Yellow);
    countdownText.setOutlineColor(sf::Color::Black);
    countdownText.setOutlineThickness(3.f);
    countdownText.setCharacterSize(std::max(80, (int)(blockSize * 4.0f)));
    countdownText.setStyle(sf::Text::Bold);

    startClock.restart();
    pausedAccumSeconds = 0.f;
    paused = false;
    state = State::Menu;

    // Load title font (HOMOARAK) separately (try both)
    std::string titlePath = findAssetPath("assets/fonts/HOMOARAK.TTF");
    if (!titlePath.empty() && titleFont.loadFromFile(titlePath)) {
        std::cout << "Loaded title font: " << titlePath << "\n";
    } else {
        std::cerr << "Failed to load title font (HOMOARAK)\n";
    }

    // Load background music if present
    // Load music using findAssetPath helper
    std::string musicPath = findAssetPath("assets/music/snow_city.mp3");
    if (!musicPath.empty() && music.openFromFile(musicPath)) {
        music.setLoop(true);
        music.setVolume(50.f);
        music.play();
        std::cout << "Background music loaded and playing: " << musicPath << "\n";
    } else {
        std::cout << "Background music not found or failed to load\n";
    }
    // Load fruit textures and high score
    loadFruitTextures();
    loadHighScore();
    // initialize fruit countdown and update tracker
    fruitCountdown = 20.f;
    lastUpdateSeconds = 0.f;
}

void GameLogic::handleInput() {
    // Only process movement input while playing and not during any countdown
    if (state == State::Playing && !showCountdown) {
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

    // Handle countdown display (3-2-1-START) and portal-specific (2-1-START)
    if (showCountdown) {
        float countdownElapsed = countdownClock.getElapsedTime().asSeconds();
        if (portalShowCountdown) {
            // Portal countdown: 2,1,START over 3 seconds
            if (countdownElapsed >= 3.0f) {
                pausedAccumSeconds += countdownClock.getElapsedTime().asSeconds();
                showCountdown = false;
                portalShowCountdown = false;
            } else if (countdownElapsed >= 2.0f) {
                countdownNumber = 0; // "START"
            } else if (countdownElapsed >= 1.0f) {
                countdownNumber = 1;
            } else {
                countdownNumber = 2;
            }
        } else {
            // Default start-game countdown: 3,2,1,START over 4 seconds
            if (countdownElapsed >= 4.0f) {
                pausedAccumSeconds += countdownClock.getElapsedTime().asSeconds();
                showCountdown = false;
            } else if (countdownElapsed >= 3.0f) {
                countdownNumber = 0; // "START"
            } else if (countdownElapsed >= 2.0f) {
                countdownNumber = 1;
            } else if (countdownElapsed >= 1.0f) {
                countdownNumber = 2;
            } else {
                countdownNumber = 3;
            }
        }
        // Don't update game while countdown is showing
        return;
    }

    // accumulate play time using startClock minus pausedAccumSeconds
    elapsedPlaySeconds = startClock.getElapsedTime().asSeconds() - pausedAccumSeconds;
    // compute delta since last update to decrement fruit countdown
    float delta = 0.f;
    if (lastUpdateSeconds <= 0.f) {
        delta = 0.f;
    } else {
        delta = elapsedPlaySeconds - lastUpdateSeconds;
        if (delta < 0.f) delta = 0.f;
    }
    lastUpdateSeconds = elapsedPlaySeconds;
    // reduce fruit countdown
    fruitCountdown -= delta;

    // Regrow while portal regrowing is active (snake is exiting and regains length)
    // Use `snake.grow()` per interval so the body emerges naturally as the head
    // advances (calling grow before update prevents tail removal on that tick).
    if (portalRegrowingActive) {
        if (portalRegrowPlaced < portalRegrowNeeded) {
            portalRegrowAccum += delta;
            if (portalRegrowAccum >= portalRegrowInterval) {
                portalRegrowAccum -= portalRegrowInterval;
                snake.grow();
                portalRegrowPlaced++;
            }
        } else {
            // Finished regrowth
            portalRegrowingActive = false;
        }
    }

    snake.update();
    Cell head = snake.getHead();
    // If stepped on a portal entrance, trigger map change and teleport
    if (portalEntrance.active && head.x == portalEntrance.x && head.y == portalEntrance.y) {
        // record old length
        int oldLen = (int)snake.getBody().size();
        // deactivate entrance
        portalEntrance.active = false;

        // Elegir primero la ubicación de salida segura y reservar área
        std::uniform_int_distribution<int> distX(barriers.getMinX() + 3, barriers.getMaxX() - 3);
        std::uniform_int_distribution<int> distY(barriers.getMinY() + 3, barriers.getMaxY() - 3);
        bool found = false;
        std::vector<Cell> safeArea;
        int ex = gridWidth / 2, ey = gridHeight / 2;
        for (int tries = 0; tries < 500 && !found; ++tries) {
            ex = distX(rng);
            ey = distY(rng);
            // Verificar que haya espacio para toda la serpiente hacia arriba
            bool ok = true;
            std::vector<Cell> tempSafe;
            for (int i = 0; i < oldLen; ++i) {
                Cell c{ex, ey - i};
                tempSafe.push_back(c);
                // Reservar también un área de 1 bloque alrededor de cada segmento
                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        Cell adj{c.x + dx, c.y + dy};
                        if (adj.x >= 0 && adj.x < gridWidth && adj.y >= 0 && adj.y < gridHeight)
                            tempSafe.push_back(adj);
                    }
                }
            }
            // Quitar duplicados
            std::sort(tempSafe.begin(), tempSafe.end(), [](const Cell&a, const Cell&b){ return a.x==b.x?a.y<b.y:a.x<b.x; });
            tempSafe.erase(std::unique(tempSafe.begin(), tempSafe.end(), [](const Cell&a, const Cell&b){ return a.x==b.x && a.y==b.y; }), tempSafe.end());
            // Comprobar que no hay paredes en el área
            for (const auto& c : tempSafe) {
                if (barriers.checkCollision(c)) { ok = false; break; }
            }
            if (ok) {
                safeArea = tempSafe;
                found = true;
            }
        }
        if (!found) {
            // Fallback: solo la cabeza y su alrededor
            ex = gridWidth / 2; ey = gridHeight / 2;
            safeArea.clear();
            for (int dx = -1; dx <= 1; ++dx)
                for (int dy = -1; dy <= 1; ++dy)
                    safeArea.push_back({ex + dx, ey + dy});
        }

        // Regenerar barreras evitando el área segura
        barriers.generateRandom(rng, gridWidth, gridHeight, safeArea);

        // Colocar la serpiente: la cabeza asoma 1 bloque arriba del portal (ey-1)
        // y el resto del cuerpo queda dentro del portal (ey, ey+1, ...).
        std::vector<Cell> nb;
        nb.push_back({ex, ey - 1});
        for (int i = 1; i < oldLen; ++i) nb.push_back({ex, ey - 1 + i});
        snake.setBody(nb);
        snake.changeDirection(0, -1);
        std::cout << "[DEBUG] Teleported to exit (" << ex << "," << ey << ") oldLen=" << oldLen << " body:";
        for (const auto &c : snake.getBody()) std::cout << " (" << c.x << "," << c.y << ")";
        std::cout << std::endl;
        portalExit.x = ex; portalExit.y = ey; portalExit.active = true; portalExit.isExit = true;
        portalTargetLength = oldLen;
        portalRegrowAccum = 0.f;
        portalRegrowingActive = false;
        portalRegrowPlaced = 0;
        portalRegrowNeeded = 0;
        // Mostrar contador para el cambio de mapa (2,1,START)
        showCountdown = true;
        portalShowCountdown = true;
        countdownClock.restart();
        fruits.clear();
        lastSpawnCheck = elapsedPlaySeconds;
        spawnFood();
        // short grace ticks to avoid immediate collision in next updates
        portalGraceTicks = 3;
    }
    
    // Si hubo teletransporte, actualizar posición de cabeza antes de comprobar colisiones
    head = snake.getHead();

    // decrement grace ticks (if any)
    if (portalGraceTicks > 0) portalGraceTicks--;

    // Si estamos mostrando el conteo del portal, no comprobamos colisiones aún.
    // Una vez acabe el conteo, `portalRegrowingActive` se activa y las colisiones
    // funcionan normalmente mientras la serpiente regresa.
    if (!portalShowCountdown && portalGraceTicks <= 0) {
        // Comprobar colisión con barreras
        if (barriers.checkCollision(head)) {
            std::cout << "[DEBUG] Collision with barrier at head (" << head.x << "," << head.y << ")" << std::endl;
            gameOver = true;
            state = State::GameOver;
            finalElapsedSeconds = startClock.getElapsedTime().asSeconds() - pausedAccumSeconds;
            baseScoreOnGameOver = score;
            timeBonusRemaining = (int)finalElapsedSeconds;
            animatedScore = baseScoreOnGameOver;
            scoreAnimationDone = false;
            scoreAnimClock.restart();
            awaitingNameEntry = false;
            return;
        }

        // Comprobar colisión consigo misma
        // Si la serpiente aún tiene segmentos dentro del portal de salida,
        // ignorar la autocolisión hasta que haya emergido por completo.
        bool hasSegmentInsidePortal = false;
        if (portalExit.active) {
            for (const auto &c : snake.getBody()) {
                if (c.y >= portalExit.y) { hasSegmentInsidePortal = true; break; }
            }
        }
        bool skipSelf = portalShowCountdown || portalGraceTicks > 0 || hasSegmentInsidePortal;
        if (!skipSelf && snake.checkSelfCollision()) {
            std::cout << "[DEBUG] Self collision detected. Head: (" << head.x << "," << head.y << ")" << std::endl;
            gameOver = true;
            state = State::GameOver;
            finalElapsedSeconds = startClock.getElapsedTime().asSeconds() - pausedAccumSeconds;
            baseScoreOnGameOver = score;
            timeBonusRemaining = (int)finalElapsedSeconds;
            animatedScore = baseScoreOnGameOver;
            scoreAnimationDone = false;
            scoreAnimClock.restart();
            awaitingNameEntry = false;
            return;
        }
    }
    
    // Check fruits eaten
    for (size_t i = 0; i < fruits.size(); ++i) {
        if (head.x == fruits[i].x && head.y == fruits[i].y) {
            // handle eating by type
            switch (fruits[i].type) {
                case Fruit::Type::Gomu:
                    score += 1;
                    snake.grow();
                    // grant time for gomu
                    fruitCountdown += 5.f;
                    // classic: when gomu eaten, spawn another gomu elsewhere
                    fruits.erase(fruits.begin() + (int)i);
                    spawnFood();
                    break;
                case Fruit::Type::Mera:
                    score += 5;
                    // grow 2 segments
                    snake.grow();
                    snake.grow();
                    fruitCountdown += 10.f;
                    fruits.erase(fruits.begin() + (int)i);
                    break;
                case Fruit::Type::Ope:
                    score += 10;
                    // grow 3 segments
                    snake.grow();
                    snake.grow();
                    snake.grow();
                    fruitCountdown += 15.f;
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

    // PORTAL: spawn entrance portal when score reaches a multiple of 30
    // Only spawn if there is no existing entrance or exit, and no portal countdown
    // or regrowth in progress. This ensures only one portal exists per map change.
    if (score >= nextPortalScore && !portalEntrance.active && !portalExit.active && !portalShowCountdown && !portalRegrowingActive) {
        // Clear all existing fruits when portal appears
        fruits.clear();
        // place entrance at random free cell
        std::uniform_int_distribution<int> distX(barriers.getMinX() + 1, barriers.getMaxX() - 1);
        std::uniform_int_distribution<int> distY(barriers.getMinY() + 1, barriers.getMaxY() - 1);
        for (int tries = 0; tries < 200; ++tries) {
            int px = distX(rng);
            int py = distY(rng);
            Cell c{px, py};
            bool ok = true;
            for (const auto &bcell : snake.getBody()) if (bcell == c) { ok = false; break; }
            if (ok && !barriers.checkCollision(c)) {
                portalEntrance.x = px; portalEntrance.y = py; portalEntrance.active = true; portalEntrance.isExit = false;
                break;
            }
        }
    }

    // Si la serpiente ya salió completamente del portal (ningún segmento
    // está en o debajo de la y de salida), quitar el portal de salida
    if (portalExit.active) {
        bool anyInside = false;
        for (const auto &c : snake.getBody()) {
            if (c.y >= portalExit.y) { anyInside = true; break; }
        }
        if (!anyInside) {
            portalExit.active = false;
            portalTargetLength = 0;
            portalRegrowingActive = false;
            nextPortalScore += 30;
        }
    }

    // if fruit countdown expired -> game over
    if (fruitCountdown <= 0.f) {
        gameOver = true;
        state = State::GameOver;
        finalElapsedSeconds = startClock.getElapsedTime().asSeconds() - pausedAccumSeconds;
        std::cout << "Game Over! Fruit timer reached zero. Score: " << score << std::endl;
        // Setup animated scoring like collision GameOver: add time bonus animation
        baseScoreOnGameOver = score;
        timeBonusRemaining = (int)finalElapsedSeconds;
        animatedScore = baseScoreOnGameOver;
        scoreAnimationDone = false;
        scoreAnimClock.restart();
        // If beat high score, defer name entry until after animation finishes
        if (score > highScore) {
            awaitingNameEntry = true;
            nameBuffer.clear();
        } else {
            awaitingNameEntry = false;
        }
        return;
    }
}

void GameLogic::draw(sf::RenderWindow& window) {
    // Dibujar barreras
    barriers.draw(window, blockSize);

    // If in menu, draw title and prompt and return
    if (state == State::Menu) {
        if (titleFont.getInfo().family.size()) {
            std::string s = "Mecha-Snake";
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
            // Controls panel in bottom-left inside the main area
            {
                // Position controls panel inside the play frame (to the left side, but not overlapping walls)
                // Use the left inner cell (minX + 1) and add a small padding
                float left = (float)(barriers.getMinX() + 1) * (float)blockSize + (float)blockSize * 0.25f;
                float bottom = (float)(gridHeight * blockSize) - (float)blockSize * 12.0f;

                sf::Text ctrlTitle("Controls:", uiFont, std::max(18, blockSize / 2));
                ctrlTitle.setFillColor(sf::Color::White);
                // Raise the Controls: title slightly higher so it doesn't overlap the key images
                // Apply same vertical offset as the control block
                float controlsYOffsetLocal = (float)blockSize * 2.0f;
                ctrlTitle.setPosition(left, bottom - (float)blockSize * 1.6f - controlsYOffsetLocal);
                window.draw(ctrlTitle);

                // Center reference X for control icons relative to the Controls: title
                sf::FloatRect ctrlBounds = ctrlTitle.getLocalBounds();
                float ctrlCenterX = left + ctrlBounds.width / 2.f;

                // Vertical positions: start a bit below the title, then leave blank lines
                float lineSpacing = (float)blockSize * 2.2f; // increased spacing to avoid overlap
                // Offset controls upward so they stay inside play frame
                float controlsYOffset = (float)blockSize * 2.0f; // move block up this much
                float yStart = bottom + (float)blockSize * 1.2f - controlsYOffset; // one 'enter' below Controls:
                float yW = yStart; // W top
                float yA = yW + lineSpacing; // A
                float yS = yW + lineSpacing * 2.0f; // S
                float yD = yW + lineSpacing * 3.0f; // D
                float yP = yW + lineSpacing * 4.0f; // P below D

                // Draw individual key images W/A/S/D and their labels as separate lines
                float keySpriteX = left + (float)blockSize * 0.6f;
                float keySize = (float)blockSize * 1.6f; // size for each key image
                auto drawKeyLine = [&](const sf::Texture &tex, const std::string &letterFallback, const std::string &label, float y){
                    if (tex.getSize().x > 0 && tex.getSize().y > 0) {
                        sf::Sprite s;
                        s.setTexture(tex);
                        sf::Vector2u ts = tex.getSize();
                        float kscale = keySize / (float)ts.x;
                        s.setScale(kscale, kscale);
                        s.setOrigin((float)ts.x / 2.f, (float)ts.y / 2.f);
                        s.setPosition(keySpriteX, y);
                        window.draw(s);

                        sf::Text lab(std::string(" -> ") + label, uiFont, std::max(16, blockSize / 2));
                        lab.setFillColor(sf::Color::White);
                        float labelX = keySpriteX + ((float)ts.x * kscale) * 0.5f + (float)blockSize * 0.2f;
                        lab.setPosition(labelX, y - (float)blockSize * 0.15f);
                        window.draw(lab);
                    } else {
                        sf::Text keyTxt(letterFallback, uiFont, std::max(20, blockSize / 2));
                        keyTxt.setFillColor(sf::Color::White);
                        sf::FloatRect kb = keyTxt.getLocalBounds();
                        keyTxt.setOrigin(kb.width / 2.f, kb.height / 2.f);
                        keyTxt.setPosition(keySpriteX, y);
                        window.draw(keyTxt);

                        sf::Text lab(std::string(" -> ") + label, uiFont, std::max(16, blockSize / 2));
                        lab.setFillColor(sf::Color::White);
                        lab.setPosition(keySpriteX + (float)blockSize * 1.0f, y - (float)blockSize * 0.15f);
                        window.draw(lab);
                    }
                };

                // Draw W, A, S, D lines
                drawKeyLine(texW, "W", "Up", yW);
                drawKeyLine(texA, "A", "Left", yA);
                drawKeyLine(texS, "S", "Down", yS);
                drawKeyLine(texD, "D", "Right", yD);

                // P pause key: show sprite then label; fallback to text if missing
                if (texP.getSize().x > 0 && texP.getSize().y > 0) {
                    sf::Sprite pSprite;
                    pSprite.setTexture(texP);
                    sf::Vector2u pts = texP.getSize();
                    // Use the same `keySize` as other key sprites, so P matches W/A/S/D
                    float pscale = keySize / (float)pts.x;
                    // Use uniform scale to preserve aspect ratio
                    pSprite.setScale(pscale, pscale);
                    pSprite.setOrigin((float)pts.x / 2.f, (float)pts.y / 2.f);
                    // Place P sprite in the left side of control block and add spacing
                    float pSpriteX = keySpriteX; // align with other keys
                    pSprite.setPosition(pSpriteX, yP);
                    window.draw(pSprite);
                    // Show arrow plus Pause label, positioned to the right of P sprite
                    sf::Text pLabel(" -> Pause", uiFont, std::max(16, blockSize / 2));
                    pLabel.setFillColor(sf::Color::White);
                    float pLabelX = pSpriteX + ((float)pts.x * pscale) * 0.5f + (float)blockSize * 0.2f;
                    pLabel.setPosition(pLabelX, yP - (float)blockSize * 0.2f);
                    window.draw(pLabel);
                } else {
                    sf::Text pText("P -> Pause", uiFont, std::max(16, blockSize / 2));
                    pText.setFillColor(sf::Color::White);
                    pText.setPosition(left + (float)blockSize * 0.6f, yP);
                    window.draw(pText);
                }
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
            // If there's an active exit portal, hide any body segments that are
            // still inside it (y >= portalExit.y) so they aren't rendered
            // until they emerge above the portal.
            if (portalExit.active && bodyCells[i].y >= portalExit.y) continue;
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

        // Dibujar portal entrance y exit usando sprite si la textura está cargada
        auto drawPortalSprite = [&](int px, int py, bool semiTransparent) {
            if (portalTexture.getSize().x > 0 && portalTexture.getSize().y > 0) {
                sf::Sprite portalSprite(portalTexture);
                float texW = (float)portalTexture.getSize().x;
                float texH = (float)portalTexture.getSize().y;
                float sizeInPixels = (float)blockSize * renderer.getSpriteScale();
                float scaleX = sizeInPixels / texW;
                float scaleY = sizeInPixels / texH;
                portalSprite.setScale(scaleX, scaleY);
                portalSprite.setOrigin(texW/2.f, texH/2.f);
                portalSprite.setPosition((float)(px * blockSize) + (float)blockSize/2.f, (float)(py * blockSize) + (float)blockSize/2.f);
                if (semiTransparent) portalSprite.setColor(sf::Color(255,255,255,120));
                window.draw(portalSprite);
            } else {
                // Fallback: rectángulo rojo
                sf::RectangleShape portal({(float)blockSize, (float)blockSize});
                portal.setFillColor(semiTransparent ? sf::Color(255,0,0,100) : sf::Color::Red);
                portal.setPosition((float)px * blockSize, (float)py * blockSize);
                window.draw(portal);
            }
        };
        if (portalEntrance.active) {
            drawPortalSprite(portalEntrance.x, portalEntrance.y, false);
        }
        if (portalExit.active) {
            drawPortalSprite(portalExit.x, portalExit.y, portalShowCountdown);
        }
    // Al final del archivo, agregar la textura de portal como miembro

    // --- Agregar miembro de textura de portal ---
    } else {
        // Fallback: dibujar rectángulos sólidos
        if (showCountdown && portalShowCountdown) {
            // During portal countdown show only the head so the body can emerge
            // from the portal tile when movement resumes.
            Cell h = snake.getHead();
            sf::RectangleShape rect({(float)blockSize, (float)blockSize});
            rect.setFillColor(sf::Color::Green);
            rect.setPosition((float)(h.x * blockSize), (float)(h.y * blockSize));
            window.draw(rect);
        } else {
            snake.draw(window, blockSize);
        }
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

    // Fruit countdown display (below main timer)
    int fsecs = (int)std::max(0.f, fruitCountdown);
    int fm = fsecs / 60;
    int fs = fsecs % 60;
    char fbuf[16];
    snprintf(fbuf, sizeof(fbuf), "%02d:%02d", fm, fs);
    fruitTimerText.setString(std::string("Fruit: ") + fbuf);
    // position just below the main timer
    float ftextW = fruitTimerText.getLocalBounds().width;
    fruitTimerText.setPosition(winW - ftextW - (float)blockSize * 0.5f, timerText.getPosition().y + timerText.getCharacterSize() + 6.f);
    {
        sf::FloatRect fb = fruitTimerText.getLocalBounds();
        sf::RectangleShape backf({fb.width + 8.f, fb.height + 8.f});
        backf.setFillColor(sf::Color(0,0,0,140));
        backf.setPosition(fruitTimerText.getPosition().x - 4.f, fruitTimerText.getPosition().y - 4.f);
        window.draw(backf);
    }
    window.draw(fruitTimerText);

    // Draw countdown (3-2-1-START) if active
    if (showCountdown) {
        std::string countdownStr;
        if (countdownNumber == 0) {
            countdownStr = "START!";
        } else {
            countdownStr = std::to_string(countdownNumber);
        }
        countdownText.setString(countdownStr);
        sf::FloatRect cbounds = countdownText.getLocalBounds();
        countdownText.setOrigin(cbounds.width / 2.f, cbounds.height / 2.f);
        float centerX = (float)(gridWidth * blockSize) / 2.f;
        float centerY = (float)(gridHeight * blockSize) / 2.f;
        countdownText.setPosition(centerX, centerY);
        window.draw(countdownText);
    }

    // Portal countdown is rendered by the central `showCountdown` block.

    // If paused, draw blinking PAUSE text in center using titleFont and show resume keys
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
                // Move PAUSE title higher — align like Game Over (quarter screen height)
                float pauseTitleY = (float)(gridHeight * blockSize) / 4.f;
                // Spacing constants to control layout
                float pauseTitleToResume = (float)blockSize * 4.5f; // more space after title
                float pauseResumeToMenu = (float)blockSize * 2.6f;  // extra blank line between subtitles
                ptext.setPosition((float)(gridWidth * blockSize) / 2.f, pauseTitleY);
                window.draw(ptext);

                // Show resume instructions separated on multiple lines with titleFont
                sf::Text resumeText("Resume: Enter or P", titleFont, std::max(20, blockSize));
                resumeText.setFillColor(sf::Color::White);
                sf::FloatRect rb = resumeText.getLocalBounds();
                resumeText.setOrigin(rb.width / 2.f, rb.height / 2.f);
                // More space below title (relative to title Y)
                resumeText.setPosition((float)(gridWidth * blockSize) / 2.f, pauseTitleY + pauseTitleToResume);
                window.draw(resumeText);

                // Show exit instruction on separate line below with more space
                sf::Text menuText("Backspace: Menu", titleFont, std::max(20, blockSize));
                menuText.setFillColor(sf::Color::White);
                sf::FloatRect mb = menuText.getLocalBounds();
                menuText.setOrigin(mb.width / 2.f, mb.height / 2.f);
                // Extra blank line between resume and menu (relative to resume position)
                menuText.setPosition((float)(gridWidth * blockSize) / 2.f, pauseTitleY + pauseTitleToResume + pauseResumeToMenu);
                window.draw(menuText);
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

        // Animated scoring: base score + time bonus counts up (Mario-style)
        if (!scoreAnimationDone) {
            // advance animation at fixed tick
            float saTick = 0.02f;
            if (scoreAnimClock.getElapsedTime().asSeconds() >= saTick) {
                scoreAnimClock.restart();
                if (timeBonusRemaining > 0) {
                    // increment animated score by 1 and decrement bonus
                    animatedScore += 1;
                    timeBonusRemaining -= 1;
                } else {
                    // animation finished
                    scoreAnimationDone = true;
                    score = animatedScore; // commit final score
                    // if record, start name entry
                    if (score > highScore) {
                        awaitingNameEntry = true;
                        nameBuffer.clear();
                    }
                }
            }

            // draw animated score and remaining time bonus
            sf::Text finalScore(std::string("Score: ") + std::to_string(animatedScore), uiFont, scoreText.getCharacterSize());
            finalScore.setFillColor(sf::Color::White);
            sf::FloatRect sb = finalScore.getLocalBounds();
            finalScore.setOrigin(sb.width / 2.f, sb.height / 2.f);
            finalScore.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f - 40.f);
            window.draw(finalScore);

            sf::Text bonusText(std::string("Time Bonus: ") + std::to_string(timeBonusRemaining) + std::string(" s"), uiFont, timerText.getCharacterSize());
            bonusText.setFillColor(sf::Color::White);
            sf::FloatRect bt = bonusText.getLocalBounds();
            bonusText.setOrigin(bt.width / 2.f, bt.height / 2.f);
            bonusText.setPosition((float)(gridWidth * blockSize) / 2.f, (float)(gridHeight * blockSize) / 2.f);
            window.draw(bonusText);
        } else {
            // final static display
            sf::Text finalScore(std::string("Score: ") + std::to_string(score), uiFont, scoreText.getCharacterSize());
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
    // generate initial random internal walls (avoid snake start cells)
    barriers.generateRandom(rng, gridWidth, gridHeight, snake.getBody());
    spawnFood();
    // reset fruit timer and portals
    fruitCountdown = 20.f;
    lastUpdateSeconds = 0.f;
    portalEntrance.active = false;
    portalExit.active = false;
    portalTargetLength = 0;
    nextPortalScore = 30;
    portalExitCountdownActive = false;
    portalExitCountdown = 0.f;
    portalShowCountdown = false;
    portalRegrowingActive = false;
    portalRegrowPlaced = 0;
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
        // Resume - show countdown again
        pausedAccumSeconds += pauseClock.getElapsedTime().asSeconds();
        state = State::Playing;
        showCountdown = true;
        countdownNumber = 3;
        countdownClock.restart();
    }
}

void GameLogic::goToMenu() {
    state = State::Menu;
    awaitingNameEntry = false;
    // reset some UI clocks
    startClock.restart();
    pausedAccumSeconds = 0.f;
    showCountdown = false;
}

void GameLogic::startGame() {
    state = State::Playing;
    startClock.restart();
    pausedAccumSeconds = 0.f;
    lastSpawnCheck = 0.f;
    snake.reset(gridWidth / 2, gridHeight / 2);
    fruits.clear();
    // generate initial random internal walls and place food
    barriers.generateRandom(rng, gridWidth, gridHeight, snake.getBody());
    spawnFood();
    score = 0;
    gameOver = false;
    finalElapsedSeconds = 0.f;
    fruitCountdown = 20.f;
    lastUpdateSeconds = 0.f;
    portalEntrance.active = false;
    portalExit.active = false;
    // inPortalMode eliminado
    portalTargetLength = 0;
    nextPortalScore = 30;
    portalExitCountdownActive = false;
    portalExitCountdown = 0.f;
    portalShowCountdown = false;
    portalRegrowingActive = false;
    portalRegrowPlaced = 0;
    
    // Start countdown timer
    showCountdown = true;
    countdownNumber = 3;
    countdownClock.restart();
}

void GameLogic::toggleTailRotate() {
    renderer.setTailRotate180(!renderer.getTailRotate180());
}

void GameLogic::loadFruitTextures() {
    auto findAssetPath = [&](const std::string &p)->std::string {
        std::ifstream f(p);
        if (f.good()) { f.close(); return p; }
        if (p.rfind("../", 0) == 0) {
            std::string alt = p.substr(3);
            std::ifstream f2(alt);
            if (f2.good()) { f2.close(); return alt; }
        } else {
            std::string alt = std::string("../") + p;
            std::ifstream f2(alt);
            if (f2.good()) { f2.close(); return alt; }
        }
        return std::string();
    };

    std::string p;
    p = findAssetPath("assets/images/gomu_gomu.png");
    if (!p.empty()) {
        if (texGomu.loadFromFile(p)) std::cout << "Loaded texture: " << p << "\n";
        else std::cerr << "Failed to load texture file (even though found): " << p << "\n";
    } else std::cerr << "gomu_gomu.png not found in candidates\n";

    p = findAssetPath("assets/images/mera_mera.png");
    if (!p.empty()) {
        if (texMera.loadFromFile(p)) std::cout << "Loaded texture: " << p << "\n";
        else std::cerr << "Failed to load texture file (even though found): " << p << "\n";
    } else std::cerr << "mera_mera.png not found in candidates\n";

    p = findAssetPath("assets/images/ope_ope.png");
    if (!p.empty()) {
        if (texOpe.loadFromFile(p)) std::cout << "Loaded texture: " << p << "\n";
        else std::cerr << "Failed to load texture file (even though found): " << p << "\n";
    } else std::cerr << "ope_ope.png not found in candidates\n";

    // Load UI control key textures (W/A/S/D and P)
    // Load individual key images W/A/S/D
    p = findAssetPath("assets/images/W.png");
    if (!p.empty()) {
        if (texW.loadFromFile(p)) std::cout << "Loaded texture: " << p << "\n";
        else std::cerr << "Failed to load texture file (even though found): " << p << "\n";
    } else std::cerr << "W.png not found in candidates\n";

    p = findAssetPath("assets/images/A.png");
    if (!p.empty()) {
        if (texA.loadFromFile(p)) std::cout << "Loaded texture: " << p << "\n";
        else std::cerr << "Failed to load texture file (even though found): " << p << "\n";
    } else std::cerr << "A.png not found in candidates\n";

    p = findAssetPath("assets/images/S.png");
    if (!p.empty()) {
        if (texS.loadFromFile(p)) std::cout << "Loaded texture: " << p << "\n";
        else std::cerr << "Failed to load texture file (even though found): " << p << "\n";
    } else std::cerr << "S.png not found in candidates\n";

    p = findAssetPath("assets/images/D.png");
    if (!p.empty()) {
        if (texD.loadFromFile(p)) std::cout << "Loaded texture: " << p << "\n";
        else std::cerr << "Failed to load texture file (even though found): " << p << "\n";
    } else std::cerr << "D.png not found in candidates\n";

    p = findAssetPath("assets/images/P.png");
    if (!p.empty()) {
        if (texP.loadFromFile(p)) std::cout << "Loaded texture: " << p << "\n";
        else std::cerr << "Failed to load texture file (even though found): " << p << "\n";
    } else std::cerr << "P.png not found in candidates\n";
}

void GameLogic::spawnCheck(float nowSeconds) {
    std::uniform_real_distribution<float> dist01(0.f, 1.f);
    const float pMera = 0.40f; // Incrementado para mayor frecuencia
    const float pOpe  = 0.15f; // Incrementado para mayor frecuencia

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
                if (barriers.checkCollision({f.x, f.y})) valid = false;
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
    // Do not spawn sporadic fruits if a portal is active or a portal countdown/show is active
    if (!portalEntrance.active && !portalExit.active && !showCountdown) {
        if (dist01(rng) < pMera) trySpawn(Fruit::Type::Mera, 4.f);
        if (dist01(rng) < pOpe)  trySpawn(Fruit::Type::Ope, 2.f);
    }
}

void GameLogic::removeExpired(float nowSeconds) {
    fruits.erase(std::remove_if(fruits.begin(), fruits.end(), [&](const Fruit &f){
        if (f.duration <= 0.f) return false;
        return (nowSeconds - f.spawnTime) >= f.duration;
    }), fruits.end());
}

void GameLogic::processEvent(const sf::Event& event) {
    // Allow some global keys even when not entering name
    if (event.type == sf::Event::KeyPressed) {
        if (!awaitingNameEntry) {
            // Ctrl+R to reset stats
            if (event.key.code == sf::Keyboard::R && event.key.control) {
                highScore = 0;
                highName = "Nobody";
                saveHighScore();
                return;
            }

            // Backspace to return to menu when in GameOver or Paused
            if (event.key.code == sf::Keyboard::BackSpace && (state == State::GameOver || state == State::Paused)) {
                goToMenu();
                return;
            }
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
                // store name uppercase
                std::string up = nameBuffer;
                for (char &c : up) c = (char)std::toupper((unsigned char)c);
                highName = up;
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
    // ensure highName is uppercase before saving
    std::string up = highName;
    for (char &c : up) c = (char)std::toupper((unsigned char)c);
    std::ofstream out("highscore.txt");
    if (!out) return;
    out << highScore << "\n";
    out << up << "\n";
}
