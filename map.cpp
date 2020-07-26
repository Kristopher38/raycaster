#include "map.h"
#include "json.hpp"
#include <fstream>

const ParsedEnum<std::string, Map::ORIENTATION> Map::orientationEnum{{
    {"none", Map::ORIENTATION::NONE},
    {"orthogonal", Map::ORIENTATION::ORTHOGONAL},
    {"isometric", Map::ORIENTATION::ISOMETRIC},
    {"staggered", Map::ORIENTATION::STAGGERED},
    {"hexagonal", Map::ORIENTATION::HEXAGONAL}
}};

const ParsedEnum<std::string, Map::RENDER_ORDER> Map::renderOrderEnum{{
    {"none", Map::RENDER_ORDER::NONE},
    {"left-up", Map::RENDER_ORDER::LEFT_UP},
    {"left-down", Map::RENDER_ORDER::LEFT_DOWN},
    {"right-up", Map::RENDER_ORDER::RIGHT_UP},
    {"right-down", Map::RENDER_ORDER::RIGHT_DOWN}
}};

const ParsedEnum<std::string, Map::TYPE> Map::typeEnum{{
    {"none", Map::TYPE::NONE},
    {"map", Map::TYPE::MAP},
    {"tileset", Map::TYPE::TILESET}
}};

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
    this->type = typeEnum.parse(json.value("type", "none"));
    switch(this->type)
    {
        case Map::TYPE::MAP:
        {
            this->mapSize.x = json.value("width", 0);
            this->mapSize.y = json.value("height", 0);
            this->tileSize.x = json.value("tilewidth", 0);
            this->tileSize.y = json.value("tileheight", 0);
            this->tiledVersion = json.value("tiledversion", "");
            this->version = json.value("version", 0.0f);
            this->compressionLevel = json.value("compressionlevel", -1);
            this->infinite = json.value("infinite", !(this->mapSize.x && this->mapSize.y));
            this->nextLayerId = json.value("nextlayerid", 0);
            this->nextObjectId = json.value("nextobjectid", 0);
            this->orientation = orientationEnum.parse(json.value("orientation", "none"));
            this->renderOrder = renderOrderEnum.parse(json.value("renderorder", "none"));


            std::vector<int32_t> mapGrid = json["layers"][0]["data"];
            for (std::size_t i = 0; i < mapGrid.size(); i += this->size().x)
                this->grid.push_back(std::vector<int32_t>(mapGrid.begin() + i, mapGrid.begin() + i + this->size().x));
            nlohmann::json tileData = json["tilesets"][0]["tiles"];
            this->tiles.reserve(tileData.size());
            for (auto& tile : tileData)
                this->tiles.push_back(std::make_unique<olc::Sprite>(tile["image"].get<std::string>()));
            break;
        }
        case Map::TYPE::TILESET:
        case Map::TYPE::NONE:
            throw std::logic_error("Invalid or not implemented map type");
    }
}


bool Map::hasWall(Vec2i coords)
{
    return this->inGridRange(coords) && this->grid[coords.y][coords.x] != 0;
}

bool Map::inGridRange(Vec2i coords)
{
    return coords.y >= 0 && coords.y < this->grid.size() && coords.x >= 0 && coords.x < this->grid[coords.y].size();
}

const Vec2i& Map::size()
{
    return this->mapSize;
}
