#ifndef MAPLAYER_H
#define MAPLAYER_H
#include "parsedenum.h"
#include "typedefs.h"
#include "olcPixelGameEngine.h"
#include "json.hpp"
#include <string>

class MapLayer
{
public:
	enum class TYPE {NONE, TILE, OBJECT, IMAGE, GROUP};

public:
    static const ParsedEnum<std::string, TYPE> typeEnum;

public:
	int32_t id;
	std::string name;
	double opacity;
	TYPE type;
	bool visible;
    Vec2i pos;
    Color tintColor;
    Vec2i offset;

    MapLayer(const nlohmann::json& json);
    virtual ~MapLayer();

    virtual void loadJson(const nlohmann::json& json);
};

class TileLayer : public MapLayer
{
public:
	std::vector<int32_t> data;
    Vec2i size;

    TileLayer(const nlohmann::json& json);
    ~TileLayer();

    int32_t& operator[](Vec2i coords);

    void loadJson(const nlohmann::json& json);
    bool hasWall(Vec2i coords);
    bool inGridRange(Vec2i coords);
};

class ObjectLayer : public MapLayer
{

};

class ImageLayer : public MapLayer
{
public:

};

class GroupLayer : public MapLayer
{
public:
    std::vector<MapLayer> layers;
};

#endif // MAPLAYER_H
