#include "map.h"

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

Map::Map()
{

}

Map::Map(const nlohmann::json& json)
{
    this->loadJson(json);
}

olc::Sprite* Map::getTile(int32_t tileIndex)
{
    return this->tiles.at(tileIndex).get();
}

void Map::loadJson(const nlohmann::json& json)
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

            for (auto layer : json.value("layers", nlohmann::json::array()))
            {
                MapLayer::TYPE layerType = MapLayer::typeEnum.parse(layer.value("type", "none"));
                switch (layerType)
                {
                    case MapLayer::TYPE::TILE:
                        this->layers.push_back(std::make_shared<TileLayer>(layer));
                        break;
                    /*case MapLayer::TYPE::OBJECT:
                        this->layers.push_back(std::make_shared<ObjectLayer>(json));
                        break;
                    case MapLayer::TYPE::IMAGE:
                        this->layers.push_back(std::make_shared<ImageLayer>(json));
                        break;
                    case MapLayer::TYPE::GROUP:
                        this->layers.push_back(std::make_shared<GroupLayer>(json));
                        break;*/
                    case MapLayer::TYPE::NONE:
                    default:
                        break;
                }
            }
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

const Vec2i& Map::size()
{
    return this->mapSize;
}
