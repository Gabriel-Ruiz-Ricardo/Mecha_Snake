#include "SnakeRenderer.hpp"
#include <iostream>
#include <fstream>

SnakeRenderer::SnakeRenderer() = default;

bool SnakeRenderer::loadTexture(sf::Texture& tex, const std::string& path) {
    auto findAsset = [&](const std::string &p)->std::string {
        // try exact
        std::ifstream f(p);
        if (f.good()) { f.close(); return p; }
        // if starts with ../ try without it
        if (p.rfind("../", 0) == 0) {
            std::string alt = p.substr(3);
            std::ifstream f2(alt);
            if (f2.good()) { f2.close(); return alt; }
        }
        // try with ../ prefix if not present
        if (p.rfind("../", 0) != 0) {
            std::string alt = std::string("../") + p;
            std::ifstream f2(alt);
            if (f2.good()) { f2.close(); return alt; }
        }
        return std::string();
    };

    std::string candidate = findAsset(path);
    if (candidate.empty()) {
        std::cerr << "Failed to find texture file: " << path << '\n';
        return false;
    }
    if (!tex.loadFromFile(candidate)) {
        std::cerr << "Failed to load texture from: " << candidate << '\n';
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

static void drawSprite(sf::RenderWindow& window, const sf::Texture& tex, int x, int y, int blockSize, float spriteScale) {
    sf::Sprite sprite(tex);
    float texW = (float)tex.getSize().x;
    float texH = (float)tex.getSize().y;
    float sizeInPixels = (float)blockSize * spriteScale;
    float scaleX = sizeInPixels / texW;
    float scaleY = sizeInPixels / texH;
    sprite.setScale(scaleX, scaleY);
    sprite.setOrigin(texW/2.f, texH/2.f);
    sprite.setPosition((float)(x * blockSize) + (float)blockSize/2.f, (float)(y * blockSize) + (float)blockSize/2.f);
    window.draw(sprite);
}

static void drawSpriteWithRotation(sf::RenderWindow& window, const sf::Texture& tex, int x, int y, int blockSize, int dirX, int dirY, float spriteScale, float extraRotation = 0.f) {
    sf::Sprite sprite(tex);
    float texW = (float)tex.getSize().x;
    float texH = (float)tex.getSize().y;
    float sizeInPixels = (float)blockSize * spriteScale;
    float scaleX = sizeInPixels / texW;
    float scaleY = sizeInPixels / texH;
    
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
    sprite.setRotation(rotation + extraRotation);
    
    // Ajustar posición para flip/rotación
    float centerX = (float)(x * blockSize) + (float)blockSize / 2.f;
    float centerY = (float)(y * blockSize) + (float)blockSize / 2.f;
    sprite.setOrigin(texW / 2.f, texH / 2.f);
    sprite.setPosition(centerX, centerY);
    
    window.draw(sprite);
}

void SnakeRenderer::drawHead(sf::RenderWindow& window, int x, int y, int blockSize, int dirX, int dirY) {
    if (!loaded) return;
    drawSpriteWithRotation(window, headTexture, x, y, blockSize, dirX, dirY, spriteScale);
}

void SnakeRenderer::drawBody(sf::RenderWindow& window, int x, int y, int blockSize, int dirX, int dirY) {
    if (!loaded) return;
    // Rotate the body sprite according to the local direction between neighboring segments.
    // This will make horizontal segments display correctly (they were appearing vertical).
    drawSpriteWithRotation(window, bodyTexture, x, y, blockSize, dirX, dirY, spriteScale);
}

void SnakeRenderer::drawTail(sf::RenderWindow& window, int x, int y, int blockSize, int dirX, int dirY) {
    if (!loaded) return;
    // Draw tail with configurable extra rotation (180 if enabled)
    float extra = tailRotate180 ? 180.f : 0.f;
    drawSpriteWithRotation(window, tailTexture, x, y, blockSize, dirX, dirY, spriteScale, extra);
}
