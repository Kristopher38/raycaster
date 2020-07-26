#ifndef PLAYER_H
#define PLAYER_H
#include "typedefs.h"

class Player
{
public:
    Vec2d pos;
    Vec2d dir;
    Player();
    Player(Vec2d _pos, Vec2d _dir);
    void move(double amount);
    void rotate(double angle);
};

#endif // PLAYER_H
