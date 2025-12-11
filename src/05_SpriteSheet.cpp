#include "SpriteSheet.hpp"
#include <iostream>

bool SpriteSheet::loadFromFile(const std::string& path) {
    sf::Image image;
    if (!image.loadFromFile(path)) {
        std::cerr << "Failed to load sprite sheet: " << path << '\n';
        loaded = false;
        return false;
    }

    // create texture
    if (!texture.loadFromImage(image)) {
        std::cerr << "Failed to create texture from image: " << path << '\n';
        loaded = false;
        return false;
    }

    // detect frames using transparency
    detectFramesByTransparency(image);
    loaded = true;
    return true;
}

// Detect vertical segments of non-transparent pixels and build frames
void SpriteSheet::detectFramesByTransparency(const sf::Image& image) {
    frames.clear();
    unsigned int W = image.getSize().x;
    unsigned int H = image.getSize().y;

    // A column is empty if every pixel has alpha == 0
    std::vector<bool> emptyCol(W, true);
    for (unsigned int x = 0; x < W; ++x) {
        for (unsigned int y = 0; y < H; ++y) {
            if (image.getPixel(x, y).a != 0) {
                emptyCol[x] = false;
                break;
            }
        }
    }

    // Find contiguous non-empty column segments
    unsigned int x = 0;
    while (x < W) {
        // skip empty columns
        while (x < W && emptyCol[x]) ++x;
        if (x >= W) break;
        unsigned int start = x;
        while (x < W && !emptyCol[x]) ++x;
        unsigned int end = x - 1;

        // For this segment, compute minY and maxY of non-transparent pixels
        unsigned int minY = H - 1;
        unsigned int maxY = 0;
        bool found = false;
        for (unsigned int xi = start; xi <= end; ++xi) {
            for (unsigned int y2 = 0; y2 < H; ++y2) {
                if (image.getPixel(xi, y2).a != 0) {
                    if (y2 < minY) minY = y2;
                    if (y2 > maxY) maxY = y2;
                    found = true;
                }
            }
        }
        if (!found) continue;

        // trim transparent rows inside bbox
        // expand 1px margin to be safe
        int frameX = (int)start;
        int frameY = (int)minY;
        int frameW = (int)(end - start + 1);
        int frameH = (int)(maxY - minY + 1);

        // Push detected frame
        frames.emplace_back(frameX, frameY, frameW, frameH);
    }

    // If frames detected, try to sort them left-to-right (already are). If none, fallback: entire image
    if (frames.empty()) {
        frames.emplace_back(0, 0, (int)W, (int)H);
    }
}

void SpriteSheet::drawDebugFrames(sf::RenderWindow& window, int blockSize) const {
    if (!loaded || frames.empty()) return;

    // Draw frame rectangles on screen
    for (size_t i = 0; i < frames.size(); ++i) {
        const auto& rect = frames[i];
        sf::RectangleShape outline({(float)rect.width, (float)rect.height});
        outline.setPosition((float)rect.left, (float)rect.top);
        outline.setFillColor(sf::Color::Transparent);
        outline.setOutlineColor(sf::Color::Yellow);
        outline.setOutlineThickness(2.f);
        window.draw(outline);

        // Draw frame number
        sf::Text frameNum(std::to_string(i), sf::Font());
        frameNum.setPosition((float)rect.left, (float)rect.top);
        frameNum.setCharacterSize(12);
        frameNum.setFillColor(sf::Color::Yellow);
        // Note: need font; if not available, skip text
    }
}
