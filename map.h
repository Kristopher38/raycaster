#ifndef MAP_H
#define MAP_H
#include <string>
#include <memory>
#include <unordered_map>
#include "olcPixelGameEngine.h"
#include <vector>
#include "json.hpp"
#include "typedefs.h"
#include "parsedenum.h"
#include "maplayer.h"
#include <fstream>

class Map
{
public:
    enum class ORIENTATION {NONE, ORTHOGONAL, ISOMETRIC, STAGGERED, HEXAGONAL};
    enum class RENDER_ORDER {NONE, LEFT_UP, LEFT_DOWN, RIGHT_UP, RIGHT_DOWN};
    enum class TYPE {NONE, MAP, TILESET};

private:
    static const ParsedEnum<std::string, ORIENTATION> orientationEnum;
    static const ParsedEnum<std::string, RENDER_ORDER> renderOrderEnum;
    static const ParsedEnum<std::string, TYPE> typeEnum;

private:
    int32_t compressionLevel;
    bool infinite;
    int32_t nextLayerId;
    int32_t nextObjectId;
    ORIENTATION orientation;
    RENDER_ORDER renderOrder;
    std::string tiledVersion;
    Vec2i tileSize;
    TYPE type;
    double version;
    Vec2i mapSize;

	std::vector<std::unique_ptr<olc::Sprite>> tiles;
    std::vector<std::shared_ptr<MapLayer>> layers;
public:
    Map();
    Map(const nlohmann::json& json);

    template<typename T> T& getLayer(int32_t layerIndex)
    {
        std::shared_ptr<T> layer = std::dynamic_pointer_cast<T>(this->layers.at(layerIndex));
        if (layer)
            return *layer;
        else
            throw std::logic_error("Invalid layer type");
    }

    olc::Sprite* getTile(int32_t tileIndex);

    void loadJson(const nlohmann::json& json);
    const Vec2i& size();
};

#endif // MAP_H
