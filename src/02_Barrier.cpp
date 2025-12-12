#include "Barrier.hpp"
#include <iostream>
#include <fstream>

Barrier::Barrier(int minX, int minY, int maxX, int maxY)
    : minX(minX), minY(minY), maxX(maxX), maxY(maxY) {
    buildWalls();
}

void Barrier::buildWalls() {
    walls.clear();
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
    // If texture is loaded, draw with sprite; otherwise fallback to rectangles
    if (wallTexture.getSize().x > 0) {
        // Draw using texture sprite
        for (const auto& wall : walls) {
            sf::Sprite sprite(wallTexture);
            sf::Vector2u ts = wallTexture.getSize();
            float texW = (float)ts.x;
            float texH = (float)ts.y;
            float sizeInPixels = (float)blockSize;
            float scaleX = sizeInPixels / texW;
            float scaleY = sizeInPixels / texH;
            sprite.setScale(scaleX, scaleY);
            sprite.setPosition((float)(wall.x * blockSize), (float)(wall.y * blockSize));
            window.draw(sprite);
        }
    } else {
        // Fallback: draw rectangles
        sf::RectangleShape rect({(float)blockSize, (float)blockSize});
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineColor(sf::Color::White);
        rect.setOutlineThickness(1.f);
        for (const auto& wall : walls) {
            rect.setPosition((float)(wall.x * blockSize), (float)(wall.y * blockSize));
            window.draw(rect);
        }
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

// Generate internal random walls with dense patterns, closed structures, and traps
void Barrier::generateRandom(std::mt19937 &rng, int gridWidth, int gridHeight, const std::vector<Cell>& forbidden) {
    // Start with border
    buildWalls();

    // grid coordinates allowed for internal walls
    int minXg = minX + 1;
    int minYg = minY + 1;
    int maxXg = maxX - 1;
    int maxYg = maxY - 1;

    if (minXg > maxXg || minYg > maxYg) return;

    // occupancy grid
    std::vector<std::vector<bool>> occ(gridWidth, std::vector<bool>(gridHeight, false));
    for (const auto &w : walls) occ[w.x][w.y] = true;
    for (const auto &f : forbidden) if (f.x >= 0 && f.x < gridWidth && f.y >= 0 && f.y < gridHeight) occ[f.x][f.y] = true;

    // Helper to test connectivity from center (BFS)
    auto connectivity = [&](int sx, int sy)->int {
        std::vector<std::vector<char>> seen(gridWidth, std::vector<char>(gridHeight, 0));
        std::vector<Cell> q;
        if (sx < 0 || sx >= gridWidth || sy < 0 || sy >= gridHeight) return 0;
        if (occ[sx][sy]) return 0;
        q.push_back({sx, sy});
        seen[sx][sy] = 1;
        size_t idx = 0;
        while (idx < q.size()) {
            Cell c = q[idx++];
            const int dx[4] = {1,-1,0,0};
            const int dy[4] = {0,0,1,-1};
            for (int i=0;i<4;++i) {
                int nx = c.x + dx[i];
                int ny = c.y + dy[i];
                if (nx<0||ny<0||nx>=gridWidth||ny>=gridHeight) continue;
                if (seen[nx][ny]) continue;
                if (occ[nx][ny]) continue;
                seen[nx][ny]=1;
                q.push_back({nx,ny});
            }
        }
        return (int)q.size();
    };

    int gridCells = (maxXg - minXg + 1) * (maxYg - minYg + 1);
    int bestTry = 0;
    std::vector<Cell> bestWalls = walls;

    // Try multiple generations with different densities
    for (int attempt = 0; attempt < 20; ++attempt) {
        auto occLocal = occ;
        std::vector<Cell> newWalls;

        // === Generate long horizontal/vertical/diagonal lines with guaranteed gaps ===
        int numLines = std::uniform_int_distribution<int>(2, 4)(rng);
        for (int l = 0; l < numLines; ++l) {
            int dir = std::uniform_int_distribution<int>(0, 2)(rng); // 0=horizontal, 1=vertical, 2=diagonal
            
            if (dir == 0) {
                // Horizontal line - can be anywhere vertically, not just edges
                int y = std::uniform_int_distribution<int>(minYg, maxYg)(rng);
                int xStart = minXg;
                int xEnd = maxXg;
                int lineLength = xEnd - xStart + 1;
                
                // Calculate minimum gaps based on line length
                // For longer lines, need more gaps to guarantee passage
                int minGaps = std::max(2, lineLength / 10); // At least 1 gap per 10 cells
                int numGaps = std::uniform_int_distribution<int>(minGaps, minGaps + 1)(rng);
                
                std::vector<int> gapPos;
                for (int g = 0; g < numGaps; ++g) {
                    gapPos.push_back(std::uniform_int_distribution<int>(xStart, xEnd)(rng));
                }
                
                for (int x = xStart; x <= xEnd; ++x) {
                    bool isGap = false;
                    for (int gx : gapPos) if (x == gx) { isGap = true; break; }
                    if (!isGap && !occLocal[x][y]) {
                        // Check for minimum 2-cell spacing from other walls vertically
                        bool canPlace = true;
                        for (int dy = -2; dy <= 2; ++dy) {
                            if (dy == 0) continue;
                            int checkY = y + dy;
                            if (checkY >= minYg && checkY <= maxYg && occLocal[x][checkY]) {
                                canPlace = false;
                                break;
                            }
                        }
                        if (canPlace) {
                            occLocal[x][y] = true;
                            newWalls.push_back({x, y});
                        }
                    }
                }
            } else if (dir == 1) {
                // Vertical line - can be anywhere horizontally
                int x = std::uniform_int_distribution<int>(minXg, maxXg)(rng);
                int yStart = minYg;
                int yEnd = maxYg;
                int lineLength = yEnd - yStart + 1;
                
                // Calculate minimum gaps based on line length
                int minGaps = std::max(2, lineLength / 10);
                int numGaps = std::uniform_int_distribution<int>(minGaps, minGaps + 1)(rng);
                
                std::vector<int> gapPos;
                for (int g = 0; g < numGaps; ++g) {
                    gapPos.push_back(std::uniform_int_distribution<int>(yStart, yEnd)(rng));
                }
                
                for (int y = yStart; y <= yEnd; ++y) {
                    bool isGap = false;
                    for (int gy : gapPos) if (y == gy) { isGap = true; break; }
                    if (!isGap && !occLocal[x][y]) {
                        // Check for minimum 2-cell spacing from other walls horizontally
                        bool canPlace = true;
                        for (int dx = -2; dx <= 2; ++dx) {
                            if (dx == 0) continue;
                            int checkX = x + dx;
                            if (checkX >= minXg && checkX <= maxXg && occLocal[checkX][y]) {
                                canPlace = false;
                                break;
                            }
                        }
                        if (canPlace) {
                            occLocal[x][y] = true;
                            newWalls.push_back({x, y});
                        }
                    }
                }
            } else {
                // Diagonal line
                int startX = std::uniform_int_distribution<int>(minXg, maxXg)(rng);
                int startY = std::uniform_int_distribution<int>(minYg, maxYg)(rng);
                int diagDir = std::uniform_int_distribution<int>(0, 3)(rng); // 0=NE, 1=NW, 2=SE, 3=SW
                
                // Generate diagonal with gaps
                std::vector<Cell> diagCells;
                int x = startX, y = startY;
                int dx = 0, dy = 0;
                
                // Set direction vectors
                if (diagDir == 0) { dx = 1; dy = -1; } // NE
                else if (diagDir == 1) { dx = -1; dy = -1; } // NW
                else if (diagDir == 2) { dx = 1; dy = 1; } // SE
                else { dx = -1; dy = 1; } // SW
                
                // Collect diagonal cells
                while (x >= minXg && x <= maxXg && y >= minYg && y <= maxYg) {
                    diagCells.push_back({x, y});
                    x += dx;
                    y += dy;
                }
                
                // Add gaps to diagonal
                int diagLength = (int)diagCells.size();
                int minGaps = std::max(1, diagLength / 8); // Fewer gaps for diagonals
                int numGaps = std::uniform_int_distribution<int>(minGaps, minGaps + 1)(rng);
                
                std::vector<int> gapIndices;
                for (int g = 0; g < numGaps && diagLength > 0; ++g) {
                    gapIndices.push_back(std::uniform_int_distribution<int>(0, diagLength - 1)(rng));
                }
                
                for (int i = 0; i < (int)diagCells.size(); ++i) {
                    bool isGap = false;
                    for (int gi : gapIndices) if (i == gi) { isGap = true; break; }
                    if (!isGap && !occLocal[diagCells[i].x][diagCells[i].y]) {
                        occLocal[diagCells[i].x][diagCells[i].y] = true;
                        newWalls.push_back(diagCells[i]);
                    }
                }
            }
        }

        // === Add scattered single pillars (very sparse) ===
        int numPillars = std::uniform_int_distribution<int>(2, 5)(rng);
        for (int p = 0; p < numPillars; ++p) {
            int px = std::uniform_int_distribution<int>(minXg, maxXg)(rng);
            int py = std::uniform_int_distribution<int>(minYg, maxYg)(rng);
            // Only single cell pillars (no 2x2)
            if (px >= minXg && px <= maxXg && py >= minYg && py <= maxYg && !occLocal[px][py]) {
                occLocal[px][py] = true;
                newWalls.push_back({px, py});
            }
        }


        // Test connectivity: need at least 70% reachable (very low density, maximum playability)
        int sx = (minXg + maxXg) / 2;
        int sy = (minYg + maxYg) / 2;
        int reachable = connectivity(sx, sy);
        if (reachable > bestTry) {
            bestTry = reachable;
            bestWalls = walls;
            for (auto &cw : newWalls) bestWalls.push_back(cw);
        }
        if (reachable >= gridCells * 0.7) {
            for (auto &cw : newWalls) walls.push_back(cw);
            return;
        }
    }

    // If no good candidate, use best
    if (bestTry > 0) {
        walls = bestWalls;
    }
}

void Barrier::loadTexture(const std::string& path) {
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
    
    std::string candidate = findAssetPath(path);
    if (!candidate.empty()) {
        if (wallTexture.loadFromFile(candidate)) {
            std::cout << "Loaded wall texture: " << candidate << "\n";
        } else {
            std::cerr << "Failed to load wall texture from: " << candidate << "\n";
        }
    } else {
        std::cerr << "Wall texture file not found: " << path << "\n";
    }
}
