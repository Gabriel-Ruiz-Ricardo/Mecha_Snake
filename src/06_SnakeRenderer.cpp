#include "SnakeRenderer.hpp"
#include <iostream>

SnakeRenderer::SnakeRenderer() = default;

bool SnakeRenderer::loadTexture(sf::Texture& tex, const std::string& path) {
    if (!tex.loadFromFile(path)) {
        std::cerr << "Failed to load texture: " << path << '\n';
        return false;
    }
    return true;
}

bool SnakeRenderer::loadSprites() {
    bool allLoaded = true;

    // Load head
    if (!loadTexture(headTexture, "../assets/images/weedle_head.png")) {
        allLoaded = false;
    }

    // Load body (single sprite)
    if (!loadTexture(bodyTexture, "../assets/images/weedle_body.png")) {
        allLoaded = false;
    }

    // Load tail
    if (!loadTexture(tailTexture, "../assets/images/weedle_tail.png")) {
        allLoaded = false;
    }

    if (allLoaded) {
        std::cout << "SnakeRenderer: All sprites loaded successfully\n";
    }
    loaded = allLoaded;
    return allLoaded;
}

static void drawSprite(sf::RenderWindow& window, const sf::Texture& tex, int x, int y, int blockSize) {
    sf::Sprite sprite(tex);
    float texW = (float)tex.getSize().x;
    float texH = (float)tex.getSize().y;
    float scaleX = (float)blockSize / texW;
    float scaleY = (float)blockSize / texH;
    sprite.setScale(scaleX, scaleY);
    sprite.setPosition((float)(x * blockSize), (float)(y * blockSize));
    window.draw(sprite);
}

static void drawSpriteWithRotation(sf::RenderWindow& window, const sf::Texture& tex, int x, int y, int blockSize, int dirX, int dirY) {
    sf::Sprite sprite(tex);
    float texW = (float)tex.getSize().x;
    float texH = (float)tex.getSize().y;
    float scaleX = (float)blockSize / texW;
    float scaleY = (float)blockSize / texH;
    
    float rotation = 0.f;
    bool flipX = false;
    
    // Determinar rotación según dirección
    if (dirX == 0 && dirY == -1) {
        // Arriba (default, no rotar)
        rotation = 0.f;
    } else if (dirX == 0 && dirY == 1) {
        // Abajo (180 grados)
        rotation = 180.f;
    } else if (dirX == 1 && dirY == 0) {
        // Derecha (90 grados + flip horizontal para mirror)
        rotation = 90.f;
        flipX = true;
    } else if (dirX == -1 && dirY == 0) {
        // Izquierda (270 grados / -90 grados)
        rotation = 270.f;
    }
    
    sprite.setScale(flipX ? -scaleX : scaleX, scaleY);
    sprite.setRotation(rotation);
    
    // Ajustar posición para flip/rotación
    float centerX = (float)(x * blockSize) + (float)blockSize / 2.f;
    float centerY = (float)(y * blockSize) + (float)blockSize / 2.f;
    sprite.setOrigin(texW / 2.f, texH / 2.f);
    sprite.setPosition(centerX, centerY);
    
    window.draw(sprite);
}

void SnakeRenderer::drawHead(sf::RenderWindow& window, int x, int y, int blockSize, int dirX, int dirY) {
    if (!loaded) return;
    drawSpriteWithRotation(window, headTexture, x, y, blockSize, dirX, dirY);
}

void SnakeRenderer::drawBody(sf::RenderWindow& window, int x, int y, int blockSize, int dirX, int dirY) {
    if (!loaded) return;
    // Rotate the body sprite according to the local direction between neighboring segments.
    // This will make horizontal segments display correctly (they were appearing vertical).
    drawSpriteWithRotation(window, bodyTexture, x, y, blockSize, dirX, dirY);
}

void SnakeRenderer::drawTail(sf::RenderWindow& window, int x, int y, int blockSize, int dirX, int dirY) {
    if (!loaded) return;
    drawSpriteWithRotation(window, tailTexture, x, y, blockSize, dirX, dirY);
}
