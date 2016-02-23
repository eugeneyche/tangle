#pragma once

#include "tile.hpp"
#include <vector>

class Tile;

class Board
{
public:
    Board(int width, int height);

    int getWidth() const;
    int getHeight() const;

    bool isOnGrid(int i, int j) const;
    
    Tile* getTile(int i, int j) const;
    Tile* getTileInDirection(Direction dir, int i, int j) const;
    Tile* getTileInAdjacentPosition(Position pos, int i, int j) const;

    bool setTile(Tile* p_tile, int i, int j);

private:
    int width_;
    int height_;
    std::vector<Tile*> grid_;
};
