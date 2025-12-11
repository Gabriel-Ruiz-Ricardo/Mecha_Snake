#include "Barrier.hpp"

Barrier::Barrier(int minX, int minY, int maxX, int maxY)
    : minX(minX), minY(minY), maxX(maxX), maxY(maxY) {
    buildWalls();
}

void Barrier::buildWalls() {
    // Pared superior
    for (int x = minX; x <= maxX; ++x) {
        walls.push_back({x, minY});
    }
    
    // Pared inferior
    for (int x = minX; x <= maxX; ++x) {
        walls.push_back({x, maxY});
    }
    
    // Pared izquierda
    for (int y = minY; y <= maxY; ++y) {
        walls.push_back({minX, y});
    }
    
    // Pared derecha
    for (int y = minY; y <= maxY; ++y) {
        walls.push_back({maxX, y});
    }
}

void Barrier::draw(sf::RenderWindow& window, int blockSize) {
    sf::RectangleShape rect({(float)blockSize, (float)blockSize});
    rect.setFillColor(sf::Color::Transparent);
    rect.setOutlineColor(sf::Color::White);
    rect.setOutlineThickness(1.f); // thinner border line
    
    for (const auto& wall : walls) {
        rect.setPosition((float)(wall.x * blockSize), (float)(wall.y * blockSize));
        window.draw(rect);
    }
}

bool Barrier::checkCollision(const Cell& pos) const {
    for (const auto& wall : walls) {
        if (wall == pos) {
            return true;
        }
    }
    return false;
}
