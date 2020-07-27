#ifndef MAPLAYER_CPP
#define MAPLAYER_CPP

#include "maplayer.h"

//{NONE, TILE, OBJECT, IMAGE, GROUP}
const ParsedEnum<std::string, MapLayer::TYPE> MapLayer::typeEnum{{
    {"none", MapLayer::TYPE::NONE},
    {"tilelayer", MapLayer::TYPE::TILE},
    {"objectgroup", MapLayer::TYPE::OBJECT},
    {"imagelayer", MapLayer::TYPE::IMAGE},
    {"group", MapLayer::TYPE::GROUP}
}};

MapLayer::MapLayer(const nlohmann::json &json)
{
    this->loadJson(json);
}

MapLayer::~MapLayer() {}

void MapLayer::loadJson(const nlohmann::json &json)
{
    this->id = json.value("id", 0);
    this->name = json.value("name", "");
    this->opacity = json.value("opacity", 1.0f);
    this->type = typeEnum.parse(json.value("type", "none"));
    this->visible = json.value("visible", true);
    this->pos.x = json.value("x", 0);
    this->pos.y = json.value("y", 0);
    this->offset.x = json.value("offsetx", 0);
    this->offset.y = json.value("offsety", 0);
}

TileLayer::TileLayer(const nlohmann::json &json) : MapLayer(json)
{
    this->loadJson(json);
}

TileLayer::~TileLayer() {}

void TileLayer::loadJson(const nlohmann::json &json)
{
    this->size.x = json.value("width", 0);
    this->size.y = json.value("height", 0);
    this->data = json.value<std::vector<int32_t>>("data", std::vector<int32_t>(0, this->size.x * this->size.y));
}

bool TileLayer::hasWall(Vec2i coords)
{
    return this->inGridRange(coords) && this->data[coords.y * this->size.y + coords.x] != 0;
}

bool TileLayer::inGridRange(Vec2i coords)
{
    return coords.y >= 0 && coords.y < this->size.y && coords.x >= 0 && coords.x < this->size.x;
}

int32_t& TileLayer::operator[](Vec2i coords)
{
    if (this->inGridRange(coords))
        return this->data[coords.y * this->size.y + coords.x];
    else
        throw std::out_of_range("Map index out of range");
}


#endif // MAPLAYER_CPP
