#pragma once

struct Cell {
    int x;
    int y;
    
    bool operator==(const Cell &o) const { 
        return x == o.x && y == o.y; 
    }
};
