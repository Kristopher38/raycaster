#include "player.h"

Player::Player() : pos(Vec2d(0, 0)), dir(Vec2d(1, 0)) {}
Player::Player(Vec2d _pos, Vec2d _dir) : pos(_pos), dir(_dir) {}

void Player::move(double amount)
{
    Vec2d newPos = this->pos + this->dir * amount;
    //Vec2i newMapPos(static_cast<int64_t>(newPos.x), static_cast<int64_t>(newPos.y));
    //if (isInMapRange(newMapPos) && map[newMapPos.y][newMapPos.x] == 0)
    this->pos = newPos;
}

void Player::rotate(double angle){
    Vec2d rotated;
    rotated.x = this->dir.x * std::cos(angle) - this->dir.y * std::sin(angle);
    rotated.y = this->dir.x * std::sin(angle) + this->dir.y * std::cos(angle);
    this->dir = rotated;
}
