#pragma once

#include <vector>

enum Direction
{
    DIR_NORTH_EAST = 0,
    DIR_NORTH,
    DIR_NORTH_WEST,
    DIR_SOUTH_WEST,
    DIR_SOUTH,
    DIR_SOUTH_EAST,
    DIR_LAST
};

enum Position
{
    POS_NORTH_EAST_0 = 0,
    POS_NORTH_EAST_1,
    POS_NORTH_0,
    POS_NORTH_1,
    POS_NORTH_WEST_0,
    POS_NORTH_WEST_1,
    POS_SOUTH_WEST_0,
    POS_SOUTH_WEST_1,
    POS_SOUTH_0,
    POS_SOUTH_1,
    POS_SOUTH_EAST_0,
    POS_SOUTH_EAST_1,
    POS_LAST
};

struct Path
{
    Position begin;
    Position end;
    bool taken;
};

class Tile
{
public:
    Tile();

    int getI() const { return i_; }
    int getJ() const { return j_; }
    Direction getOrientation() const;
    Position toLocal(Position src) const;
    Position toGlobal(Position src) const;
    Position getAdjacentPosition(Position from_pos) const;
    Position getDestination(Position from_pos) const;
    Path* getPathAtPosition(Position from_pos);
    std::vector<Path>& getPaths();

    void setOrientation(Direction orientation);
    void setCoord(int i, int j) { i_ = i; j_ = j; }
    void addPath(Position pos0, Position pos1);
    void clearPaths();

    void randomlyGeneratePaths();

    bool canRotate() const;
    bool isPathTaken(Position from_pos);

    void rotateLeft();
    void rotateRight();
    Position traverse(Position from_pos);
    
private:
    int i_;
    int j_;
    Direction orientation_ = DIR_NORTH_EAST;
    Position adjacent_ [POS_LAST];
    int path_map_ [POS_LAST];
    std::vector<Path> paths_;
};
