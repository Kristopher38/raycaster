#define _USE_MATH_DEFINES
#define OLC_PGE_APPLICATION

#include <math.h>
#include <utility>
#include <vector>
#include <limits>
#include <iostream>
#include <unordered_map>

#include "olcPixelGameEngine.h"
#include "typedefs.h"
#include "player.h"
#include "tileson.hpp"

enum WALL_SIDE { NONE, VERTICAL, HORIZONTAL };

struct RaycastResult
{
    double hitPoint;
    double hitDist;
    WALL_SIDE wallSide;
    Vec2i mapPos;
};

class Raycaster : public olc::PixelGameEngine
{
private:
    const double fov = M_PI_2 * 2.0f/3.0f; // 60 degrees
    Player player = Player(Vec2d(4.3, 5.7), Vec2d(-1, 0));
    tson::Map map;
    std::map<fs::path, std::unique_ptr<olc::Sprite>> spritesData;
    std::vector<int> debugLines;

    inline Vec2d getCameraPlane(Vec2d dir)
    {
        return dir.perp().norm() * std::tan(this->fov / 2.0f) * dir.mag();
    }

    inline void clearSprite(std::unique_ptr<olc::Sprite> sprite)
    {
        this->SetDrawTarget(sprite.get());
        this->Clear(olc::BLANK);
        this->SetDrawTarget(nullptr);
    }

    inline std::unique_ptr<olc::Sprite> newLayer(uint32_t w, uint32_t h)
    {
        std::unique_ptr<olc::Sprite> sprite = std::make_unique<olc::Sprite>(w, h);
        this->SetDrawTarget(sprite.get());
        this->Clear(olc::BLANK);
        this->SetDrawTarget(nullptr);
        return sprite;
    }

    inline std::unique_ptr<olc::Sprite> newLayer()
    {
        return this->newLayer(this->ScreenWidth(), this->ScreenHeight());
    }

    void renderMap(float elapsedTime, double scale)
    {
        Vec2d cameraPlane = this->getCameraPlane(player.dir);
        Vec2d curTile(std::floor(player.pos.x) - player.pos.x, std::floor(player.pos.y) - player.pos.y);

        static auto airs = this->newLayer(map.getSize().y * scale, map.getSize().x * scale);
        static auto walls = this->newLayer(map.getSize().y * scale, map.getSize().x * scale);
        for (int32_t row = 0; row < map.getSize().y; ++row)
        {
            for (int32_t col = 0; col < map.getSize().x; ++col)
            {
                if (map.getLayer("walls")->getTileData(col, row) != nullptr)
                {
                    this->SetDrawTarget(walls.get());
                    this->FillRect(row * scale, col * scale, scale, scale, olc::Pixel(61, 201, 56)); // slightly dark green
                    this->DrawRect(row * scale, col * scale, scale, scale, olc::Pixel(72, 245, 66)); // bright green
                }
                else
                {
                    this->SetDrawTarget(airs.get());
                    this->DrawRect(row * scale, col * scale, scale, scale, olc::Pixel(38, 125, 35)); // darkest green
                }
            }
        }

        this->SetDrawTarget(nullptr);
        this->DrawSprite(0, 0, airs.get());
        this->DrawSprite(0, 0, walls.get());

        // draw camera triangle
        this->FillRect((player.pos + curTile) * scale + Vec2d(1.0, 1.0), Vec2d(scale, scale) - Vec2d(1.0, 1.0), olc::GREY);
        this->FillCircle(player.pos * scale, 0.25 * scale, olc::RED);
        this->DrawLine(player.pos * scale, (player.pos+player.dir) * scale, olc::YELLOW);
        this->DrawLine((player.pos+player.dir-cameraPlane) * scale,
                       (player.pos+player.dir+cameraPlane) * scale, olc::MAGENTA);
        this->DrawLine(player.pos * scale, (player.pos+player.dir-cameraPlane) * scale, olc::DARK_MAGENTA);
        this->DrawLine(player.pos * scale, (player.pos+player.dir+cameraPlane) * scale, olc::DARK_MAGENTA);
    }

    RaycastResult raycast(Vec2d rayDir)
    {
        Vec2d rayStepDist;
        // scale normalized rayDir by a factor that first makes x component equal to 1
        rayStepDist.x = (rayDir.norm() * 1 / rayDir.x).mag();
        // ...and second y component equal to 1, and compute their magnitudes
        rayStepDist.y = (rayDir.norm() * 1 / rayDir.y).mag();
        // based on the direction of the ray, calculate initial distances to respective x (vertical) and y (horizontal) grid intersections

        Vec2d curRayDist;
        if (rayDir.x > 0)
            curRayDist.x = (std::ceil(player.pos.x) - player.pos.x) * rayStepDist.x;
        else
            curRayDist.x = (player.pos.x - std::floor(player.pos.x)) * rayStepDist.x;
        if (rayDir.y > 0)
            curRayDist.y = (std::ceil(player.pos.y) - player.pos.y) * rayStepDist.y;
        else
            curRayDist.y = (player.pos.y - std::floor(player.pos.y)) * rayStepDist.y;

        Vec2i mapPos;
        mapPos.x = static_cast<uint32_t>(player.pos.x);
        mapPos.y = static_cast<uint32_t>(player.pos.y);
        Vec2i mapStep;
        mapStep.x = std::signbit(rayDir.x) ? -1 : 1;
        mapStep.y = std::signbit(rayDir.y) ? -1 : 1;

        WALL_SIDE side = WALL_SIDE::NONE;
        try
        {
            while (this->map.getLayer("walls")->getTileData(mapPos.x, mapPos.y) == nullptr)
            {
                if (curRayDist.x < curRayDist.y)
                {
                    curRayDist.x += rayStepDist.x;
                    mapPos.x += mapStep.x;
                    side = WALL_SIDE::VERTICAL;
                }
                else
                {
                    curRayDist.y += rayStepDist.y;
                    mapPos.y += mapStep.y;
                    side = WALL_SIDE::HORIZONTAL;
                }
            }
        }
        catch (std::out_of_range& e)
        {
            side = WALL_SIDE::NONE;
        }

        double hitDist;
        double wallHitPos;
        switch (side)
        {
            case WALL_SIDE::NONE:
                hitDist = std::numeric_limits<double>::infinity();
                wallHitPos = 0.0f;
                break;
            case WALL_SIDE::VERTICAL:
                hitDist = (mapPos.x - player.pos.x + (std::signbit(rayDir.x) ? 1 : 0)) / rayDir.x;
                wallHitPos = player.pos.y + hitDist * rayDir.y;
                break;
            case WALL_SIDE::HORIZONTAL:
                hitDist = (mapPos.y - player.pos.y + (std::signbit(rayDir.y) ? 1 : 0)) / rayDir.y;
                wallHitPos = player.pos.x + hitDist * rayDir.x;
                break;
        }

        return RaycastResult{wallHitPos, hitDist, side, mapPos};
    }

    void renderScene(float elapsedTime)
    {
        Vec2d cameraPlane = this->getCameraPlane(player.dir);

        for (int32_t col = 0; col < this->ScreenWidth(); ++col)
        {
            Vec2d rayDir = player.dir + cameraPlane * (2 * static_cast<double>(col) / static_cast<double>(this->ScreenWidth()) - 1);
            // make sure division by 0 never happens
            rayDir.x = rayDir.x == 0 ? 1e-10 : rayDir.x;
            rayDir.y = rayDir.y == 0 ? 1e-10 : rayDir.y;

            RaycastResult ray = raycast(rayDir);

            ray.hitPoint -= std::floor(ray.hitPoint);
            auto tile = this->map.getLayer("walls")->getTileData(ray.mapPos.x, ray.mapPos.y);

            if (tile)
            {
                double textureWidth = tile->getImageSize().x;
                double textureHeight = tile->getImageSize().y;
                double lineHeight = static_cast<double>(this->ScreenHeight()) / ray.hitDist;
                int32_t lineLength = lineHeight > this->ScreenHeight() ? this->ScreenHeight() : lineHeight;

                Vec2d texturePos(ray.hitPoint * textureWidth, (lineHeight - lineLength) / lineHeight / 2 * textureHeight);
                if (ray.wallSide == WALL_SIDE::VERTICAL && rayDir.x > 0)
                    texturePos.x = texturePos.x - 1;
                if (ray.wallSide == WALL_SIDE::HORIZONTAL && rayDir.y < 0)
                    texturePos.x = texturePos.x - 1;
                double textureStep = textureHeight / lineHeight;

                const auto sprite = spritesData.at(tile->getImage()).get();
                for (int32_t row = this->ScreenHeight() / 2 - lineLength / 2; row < this->ScreenHeight()/2 + lineLength/2; ++row)
                {
                    olc::Pixel p = sprite->GetPixel(texturePos);
                    texturePos.y += textureStep;
                    this->Draw(col, row, p);
                }
            }
        }
    }

    void render(float elapsedTime)
    {
        this->SetDrawTarget(nullptr);
        this->SetPixelMode(olc::Pixel::Mode::MASK);
        this->Clear(olc::BLACK);

        this->renderScene(elapsedTime);
        this->renderMap(elapsedTime, 10);
        //this->DrawSprite(0, 0, scene.get());
    }

    void handleInput(float elapsedTime)
    {
        const double turnSensitivity = 2.0;
        const double walkSensitivity = 2.5;
        if (this->GetKey(olc::Key::A).bHeld)
            player.rotate(-turnSensitivity * elapsedTime);
        if (this->GetKey(olc::Key::D).bHeld)
            player.rotate(turnSensitivity * elapsedTime);

        if (this->GetKey(olc::Key::W).bHeld)
            player.move(walkSensitivity * elapsedTime);
        if (this->GetKey(olc::Key::S).bHeld)
            player.move(-walkSensitivity * elapsedTime);
    }

public:
    Raycaster()
    {
        this->sAppName = "Raycaster";
    }
    virtual ~Raycaster() {}

    bool OnUserCreate() override
    {
        tson::Tileson parser;
        this->map = parser.parse(fs::path("maps/testmap.json"));
        if (this->map.getStatus() == tson::ParseStatus::OK)
        {
            for (auto& tileset : this->map.getTilesets())
            {
                for (auto& tile : tileset.getTiles())
                {
                    this->spritesData.emplace(tile.getImage(), std::make_unique<olc::Sprite>("maps/"+tile.getImage().string()));
                }
            }
            return true;
        }
        else
            return false;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        this->render(fElapsedTime);
        this->handleInput(fElapsedTime);
        this->SetDrawTarget(nullptr);
        return true;
    }

    bool OnUserDestroy() override
    {
        return true;
    }
};

int main()
{
    Raycaster demo;
    if (demo.Construct(640, 400, 2, 2))
        demo.Start();
    return 0;
}
