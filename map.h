#ifndef MAP_H
#define MAP_H
#include <string>
#include <memory>
#include "olcPixelGameEngine.h"
#include <vector>
#include "json.hpp"
#include "typedefs.h"

class Map
{
private:
	std::vector<std::unique_ptr<olc::Sprite>> tiles;
    std::vector<std::vector<int32_t>> grid;
public:
    int32_t width;
    int32_t height;

	Map();
    Map(std::string filename);

    int32_t& operator[](Vec2i coords);
    olc::Sprite* getTile(int32_t tileIndex);


    void loadJson(nlohmann::json json);
    bool hasWall(Vec2i coords);
    bool inGridRange(Vec2i coords);

};

#endif // MAP_H
