#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class SpriteSheet {
public:
    SpriteSheet() = default;

    bool loadFromFile(const std::string& path);
    const sf::Texture& getTexture() const { return texture; }
    const std::vector<sf::IntRect>& getFrames() const { return frames; }
    bool isLoaded() const { return loaded; }

    // Debug: dibujar rectángulos alrededor de los frames detectados
    void drawDebugFrames(sf::RenderWindow& window, int blockSize) const;

private:
    sf::Texture texture;
    std::vector<sf::IntRect> frames;
    bool loaded = false;

    void detectFramesByTransparency(const sf::Image& image);
};
