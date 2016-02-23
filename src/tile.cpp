#include "tile.hpp"
#include <ctime>
#include <algorithm>
#include <random>

Tile::Tile()
{
    clearPaths();
}

Direction Tile::getOrientation() const
{
    return orientation_;
}

Position Tile::toLocal(Position src) const
{
    return static_cast<Position>((src + 2 * (DIR_LAST - orientation_)) % (2 * DIR_LAST));
}

Position Tile::toGlobal(Position src) const
{
    return static_cast<Position>((src + 2 * orientation_) % (2 * DIR_LAST));
}

Position Tile::getAdjacentPosition(Position from_pos) const
{
    Direction from_dir = static_cast<Direction>(from_pos / 2);
    return static_cast<Position>(((from_dir + DIR_LAST / 2) % DIR_LAST) * 2 + (from_pos + 1) % 2);
}

Position Tile::getDestination(Position from_pos) const
{
    Position src = getAdjacentPosition(from_pos);
    src = toLocal(src);
    Position dst = adjacent_[src];
    return toGlobal(dst);
}

Path* Tile::getPathAtPosition(Position from_pos)
{
    Position src = toLocal(getAdjacentPosition(from_pos));
    int path_idx = path_map_[src];
    if (path_idx < 0)
        return nullptr;
    return &paths_.at(path_idx);
}

std::vector<Path>& Tile::getPaths()
{
    return paths_;
}

void Tile::setOrientation(Direction orientation) 
{
    orientation_ = orientation;
}

void Tile::addPath(Position p0, Position p1)
{
    if (p0 > p1)
    {
        Position tmp = p0;
        p0 = p1;
        p1 = tmp;
    }
    adjacent_[p0] = p1;
    adjacent_[p1] = p0;
    path_map_[p0] = paths_.size();
    path_map_[p1] = paths_.size();
    paths_.push_back({p0, p1, false});
}

void Tile::clearPaths()
{
    std::fill(adjacent_, adjacent_ + POS_LAST, POS_LAST);
    std::fill(path_map_, path_map_ + POS_LAST, -1);
    paths_.clear();
}

void Tile::randomlyGeneratePaths()
{
    static std::mt19937 rand;
    static bool is_seeded = false;
    if (!is_seeded)
    {
        rand.seed(time(NULL));
        is_seeded = true;
    }
    clearPaths();
    int avail [POS_LAST];
    for (int i = 0; i < POS_LAST; i++)
    {
        avail[i] = i;
    }
    for (int i = 0; i < POS_LAST; i++)
    {
        int next = i + rand() % (POS_LAST - i);
        int tmp = avail[i];
        avail[i] = avail[next];
        avail[next] = tmp;
    }
    for (int i = 0; i + 2 <= POS_LAST; i += 2)
    {
        Position pos0 = static_cast<Position>(avail[i]);
        Position pos1 = static_cast<Position>(avail[i + 1]);
        addPath(pos0, pos1);
    }
}

bool Tile::canRotate() const
{
    for (Path path : paths_)
    {
        if (path.taken)
            return false;
    }
    return true;
}

bool Tile::isPathTaken(Position from_pos)
{
    Path* path = getPathAtPosition(from_pos);
    if (!path)
        return false;
    return path->taken;
}

void Tile::rotateLeft()
{
    orientation_ = static_cast<Direction>((orientation_ + 1) % DIR_LAST);
}

void Tile::rotateRight()
{
    orientation_ = static_cast<Direction>((orientation_ + DIR_LAST - 1) % DIR_LAST);
}

Position Tile::traverse(Position from_pos)
{
    Path* path = getPathAtPosition(from_pos);
    if (!path)
        return from_pos;
    path->taken = true;
    return getDestination(from_pos);
}
