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

class Map
{
public:
    enum class ORIENTATION {NONE, ORTHOGONAL, ISOMETRIC, STAGGERED, HEXAGONAL};
    enum class RENDER_ORDER {NONE, LEFT_UP, LEFT_DOWN, RIGHT_UP, RIGHT_DOWN};
    enum class TYPE {NONE, MAP, TILESET};

public:
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
    std::vector<std::vector<int32_t>> grid;
public:
	Map();
    Map(std::string filename);

    int32_t& operator[](Vec2i coords);
    olc::Sprite* getTile(int32_t tileIndex);

    void loadJson(nlohmann::json json);
    bool hasWall(Vec2i coords);
    bool inGridRange(Vec2i coords);
    const Vec2i& size();
};

#endif // MAP_H
