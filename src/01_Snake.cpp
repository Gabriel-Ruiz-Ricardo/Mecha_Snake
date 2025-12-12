#include "Snake.hpp"

Snake::Snake(int startX, int startY, sf::Color col)
    : color(col), direction({0, -1}), nextDirection({0, -1}) {
    body.push_back({startX, startY});        // head
    body.push_back({startX, startY + 1});    // body
    body.push_back({startX, startY + 2});    // tail
}

void Snake::changeDirection(int dx, int dy) {
    // Evitar que la serpiente se voltee sobre sí misma
    if ((direction.x == 0 && direction.y == -1 && dy == 1) ||  // up -> down
        (direction.x == 0 && direction.y == 1 && dy == -1) ||   // down -> up
        (direction.x == -1 && direction.y == 0 && dx == 1) ||   // left -> right
        (direction.x == 1 && direction.y == 0 && dx == -1)) {   // right -> left
        return;
    }
    nextDirection = {dx, dy};
}

void Snake::update() {
    direction = nextDirection;
    
    Cell head = body.front();
    Cell newHead{head.x + direction.x, head.y + direction.y};
    
    body.insert(body.begin(), newHead);
    body.pop_back();
}

void Snake::draw(sf::RenderWindow& window, int blockSize) {
    sf::RectangleShape rect({(float)blockSize, (float)blockSize});
    rect.setFillColor(color);
    // Draw body and tail first
    for (size_t i = 1; i < body.size(); ++i) {
        rect.setPosition((float)(body[i].x * blockSize), (float)(body[i].y * blockSize));
        rect.setFillColor(color);
        window.draw(rect);
    }
    // Draw head last so it stays on top
    if (!body.empty()) {
        rect.setPosition((float)(body[0].x * blockSize), (float)(body[0].y * blockSize));
        sf::Color darkColor(color.r / 2, color.g / 2, color.b / 2);
        rect.setFillColor(darkColor);
        window.draw(rect);
    }
}

bool Snake::checkSelfCollision() const {
    Cell head = body.front();
    for (size_t i = 1; i < body.size(); ++i) {
        if (body[i] == head) {
            return true;
        }
    }
    return false;
}

void Snake::grow() {
    Cell tail = body.back();
    body.push_back(tail);
}

void Snake::setBody(const std::vector<Cell>& b) {
    body = b;
}

void Snake::shrinkTo(int len) {
    if (len < 1) len = 1;
    if ((int)body.size() <= len) return;
    body.resize((size_t)len);
}

void Snake::reset(int startX, int startY) {
    body.clear();
    body.push_back({startX, startY});        // head
    body.push_back({startX, startY + 1});    // body
    body.push_back({startX, startY + 2});    // tail
    direction = {0, -1};
    nextDirection = {0, -1};
}
