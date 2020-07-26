#include "map.h"
#include "json.hpp"
#include <fstream>

Map::Map() {}

Map::Map(std::string filename) : Map()
{
    std::ifstream jsonFile(filename);
    this->loadJson(nlohmann::json::parse(jsonFile));
}

int32_t& Map::operator[](Vec2i coords)
{
    if (this->inGridRange(coords))
        return this->grid[coords.y][coords.x];
    else
        throw std::out_of_range("Map index out of range");
}

olc::Sprite* Map::getTile(int32_t tileIndex)
{
    return this->tiles.at(tileIndex).get();
}

void Map::loadJson(nlohmann::json json)
{
    this->width = json["layers"][0]["width"];
    this->height = json["layers"][0]["height"];
    std::vector<int32_t> mapGrid = json["layers"][0]["data"];
    for (std::size_t i = 0; i < mapGrid.size(); i += this->width)
        this->grid.push_back(std::vector<int32_t>(mapGrid.begin() + i, mapGrid.begin() + i + this->width));
    nlohmann::json tileData = json["tilesets"][0]["tiles"];
    this->tiles.reserve(tileData.size());
    for (auto& tile : tileData)
        this->tiles.push_back(std::make_unique<olc::Sprite>(tile["image"].get<std::string>()));
}

bool Map::hasWall(Vec2i coords)
{
    return this->inGridRange(coords) && this->grid[coords.y][coords.x] != 0;
}

bool Map::inGridRange(Vec2i coords)
{
    return coords.y >= 0 && coords.y < this->grid.size() && coords.x >= 0 && coords.x < this->grid[coords.y].size();
}
