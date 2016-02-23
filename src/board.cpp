#include "board.hpp"

Board::Board(int width, int height)
    : width_ (width)
    , height_ (height)
    , grid_ (width_ * height_)
{ }

int Board::getWidth() const
{
    return width_;
}

int Board::getHeight() const
{
    return height_;
}

bool Board::isOnGrid(int i, int j) const
{
    bool valid_i = 0 <= i && i < height_;
    bool valid_j = 0 <= j && j < width_;
    return valid_i && valid_j;
}

Tile* Board::getTile(int i, int j) const
{
    if (!isOnGrid(i, j))
        return nullptr;
    return grid_[i * width_ + j];
}

Tile* Board::getTileInDirection(Direction d, int i, int j) const
{
    switch (d) 
    {
    case DIR_NORTH_EAST:
        i--;
        j++;
        break;
    case DIR_NORTH:
        i--;
        break;
    case DIR_NORTH_WEST:
        j--;
        break;
    case DIR_SOUTH_WEST:
        j--;
        i++;
        break;
    case DIR_SOUTH:
        i++;
        break;
    case DIR_SOUTH_EAST:
        j++;
        break;
    }
    return getTile(i, j);
}

Tile* Board::getTileInAdjacentPosition(Position p, int i, int j) const
{
    return getTileInDirection(static_cast<Direction>(p / 2), i, j);
}

bool Board::setTile(Tile* p_tile, int i, int j)
{
    if (!isOnGrid(i, j))
        return false;
    grid_[i * width_ + j] = p_tile;
    p_tile->setCoord(i, j);
    return true;
}
