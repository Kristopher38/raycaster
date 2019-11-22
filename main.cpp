#include <iostream>
#include <cmath>
#include <utility>
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

struct Vec2d
{
    public:
        int32_t x;
        int32_t y;
};

class Player
{
public:
    double angle;
    double x;
    double y;
};

class Raytracer : public olc::PixelGameEngine
{
private:
    const uint16_t gridSize = 64;
    double fov = radians(60.0);
    double distPlane;
    double rayAngleDelta;
    Player player;

    /*      180deg
     *
     * 90deg       270deg
     *
     *       0deg
     */
    uint8_t map[10][10] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    };
    std::vector<int> debugLines;

    inline double dist(Vec2d v1, Vec2d v2)
    {
        return sqrt((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y));
    }

    inline double dist(Vec2d v1, Vec2d v2, double rayAngle)
    {
        return static_cast<double>(abs(v1.x - v2.x)) / cos(rayAngle);
    }

    inline bool hasWall(Vec2d coords)
    {
        return coords.x/64 < 10 && coords.y/64 < 10 && coords.x/64 >= 0 && coords.y/64 >= 0
                && map[coords.x/64][coords.y/64] != 0;
    }

    inline double radians(double degrees)
    {
        return degrees * M_PI/180.0;
    }

    double wrapAngle(double angle)
    {
        if (angle < radians(0.0))
            angle += radians(360.0);
        else if (angle >= radians(360.0))
            angle -= radians(360.0);
        return angle;
    }

    olc::Sprite renderScene(float elapsedTime)
    {
        olc::Sprite scene(this->ScreenWidth(), this->ScreenHeight());

        this->SetDrawTarget(&scene);
        this->Clear(olc::BLANK);

        double rayAngle = player.angle - (fov / 2.0);
        for (uint16_t col = 0; col < this->ScreenWidth(); ++col)
        {
            rayAngle = this->wrapAngle(rayAngle);
            Vec2d rayIntersect;
            Vec2d delta;
            Vec2d wallHorizontal;
            Vec2d wallVertical;

            // horizontal gridlines
            if (rayAngle < radians(180.0))
                rayIntersect.y = (static_cast<int32_t>(player.y) / 64) * 64 - 1;
            else
                rayIntersect.y = (static_cast<int32_t>(player.y) / 64) * 64 + 64;
            rayIntersect.x = static_cast<int32_t>(player.x + static_cast<double>(player.y - rayIntersect.y) / tan(rayAngle));

            if (rayAngle < radians(180.0))
                delta.y = -gridSize;
            else delta.y = gridSize;

            if (rayAngle >= radians(90.0) && rayAngle < radians(270.0))
                delta.x = -abs(static_cast<int32_t>(static_cast<double>(gridSize) / tan(rayAngle)));
            else delta.x = abs(static_cast<int32_t>(static_cast<double>(gridSize) / tan(rayAngle)));

            bool outOfBoundsH = false;
            while (!hasWall(rayIntersect)
                   && rayIntersect.x >= 0
                   && rayIntersect.y >= 0
                   && rayIntersect.x < 10*64
                   && rayIntersect.y < 10*64)
            {
                rayIntersect.x += delta.x;
                rayIntersect.y += delta.y;
            }
            if (rayIntersect.x < 0 || rayIntersect.y < 0 || rayIntersect.x >= 10*64 || rayIntersect.y >= 10*64)
                outOfBoundsH = true;
            wallHorizontal = rayIntersect;

            // vertical gridlines
            if (rayAngle >= radians(90.0) && rayAngle < radians(270.0))
                rayIntersect.x = (static_cast<int32_t>(player.x)/64) * 64 - 1;
            else rayIntersect.x = (static_cast<int32_t>(player.x)/64) * 64 + 64;
            rayIntersect.y = static_cast<int32_t>(player.y + static_cast<double>(player.x - rayIntersect.x) * tan(rayAngle));

            if (rayAngle >= radians(90.0) && rayAngle < radians(270.0))
                delta.x = -gridSize;
            else delta.x = gridSize;

            if (rayAngle < radians(180.0))
                delta.y = -abs(static_cast<int32_t>(64.0 * tan(rayAngle)));
            else delta.y = abs(static_cast<int32_t>(64.0 * tan(rayAngle)));

            bool outOfBoundsV = false;
            while (!hasWall(rayIntersect)
                   && rayIntersect.x >= 0
                   && rayIntersect.y >= 0
                   && rayIntersect.x < 10*64
                   && rayIntersect.y < 10*64)
            {
                rayIntersect.x += delta.x;
                rayIntersect.y += delta.y;
            }

            if (rayIntersect.x < 0 || rayIntersect.y < 0 || rayIntersect.x >= 10*64 || rayIntersect.y >= 10*64)
                outOfBoundsV = true;
            wallVertical = rayIntersect;

            Vec2d playerpos;
            playerpos.x = player.x;
            playerpos.y = player.y;
            double distH = dist(playerpos, wallHorizontal);
            double distV = dist(playerpos, wallVertical);
            double distance;
            olc::Pixel wallColor;

            if (distH > distV && !outOfBoundsV)
            {
                distance = distV;
                wallColor = olc::GREEN;
            }
            else if (!outOfBoundsH)
            {
                distance = distH;
                wallColor = olc::DARK_GREEN;
            }
            else
                distance = std::numeric_limits<double>::infinity();

            if (distance != std::numeric_limits<double>::infinity())
            {
                double sliceHeight = (static_cast<double>(gridSize)/(distance*cos(rayAngle - player.angle))) * this->distPlane;
                this->DrawLine(col, this->ScreenHeight()/2 - static_cast<uint16_t>(sliceHeight/2), col, this->ScreenHeight()/2 + static_cast<uint16_t>(sliceHeight/2), wallColor);
            }
            rayAngle += this->rayAngleDelta;
        }

        this->SetDrawTarget(nullptr);
        return scene;
    }

    olc::Sprite renderDebug(float elapsedTime)
    {
        olc::Sprite debugInfo(this->ScreenWidth(), this->ScreenHeight());
        this->SetDrawTarget(&debugInfo);
        this->Clear(olc::BLANK);
        uint16_t textCounter = 0;

        this->DrawString(0, 8*textCounter++, std::to_string(1.0f/elapsedTime));
        this->DrawString(0, 8*textCounter++, std::to_string(player.angle * 180.0/M_PI), olc::RED);
        this->DrawString(0, 8*textCounter++, std::to_string(player.x), olc::RED);
        this->DrawString(0, 8*textCounter++, std::to_string(player.y), olc::RED);

        for (std::vector<int>::iterator it = debugLines.begin(); it != debugLines.end(); ++it)
        {
            this->DrawLine(*it, 0, *it, this->ScreenHeight(), olc::DARK_RED);
            this->DrawString(0, 8*textCounter++, std::to_string(*it), olc::RED);
        }

        this->SetDrawTarget(nullptr);
        return debugInfo;
    }

    void render(float elapsedTime)
    {
        this->SetPixelMode(olc::Pixel::Mode::MASK);
        this->Clear(olc::BLACK);

        olc::Sprite scene = this->renderScene(elapsedTime);
        this->DrawSprite(0, 0, &scene);
        olc::Sprite debugInfo = this->renderDebug(elapsedTime);
        this->DrawSprite(0, 0, &debugInfo);
    }

    void handleInput(float elapsedTime)
    {
        const double turnSensitivity = 2.0;
        const double walkSensitivity = 120.0;
        if (this->GetKey(olc::Key::A).bHeld)
            player.angle += turnSensitivity * elapsedTime;
        if (this->GetKey(olc::Key::D).bHeld)
            player.angle -= turnSensitivity * elapsedTime;
        player.angle = wrapAngle(player.angle);

        double dirx = -cos(player.angle - radians(180.0)) * walkSensitivity * elapsedTime;
        double diry = sin(player.angle - radians(180.0)) * walkSensitivity * elapsedTime;

        if (this->GetKey(olc::Key::W).bHeld)
        {
            player.x += dirx;
            player.y += diry;
        }
        if (this->GetKey(olc::Key::S).bHeld)
        {
            player.x -= dirx;
            player.y -= diry;
        }

        if (this->GetMouse(0).bPressed)
        {
            int mouseX = this->GetMouseX();
            std::vector<int>::iterator dupMouseX = std::find(debugLines.begin(), debugLines.end(), mouseX);
            if (dupMouseX == debugLines.end())
                debugLines.push_back(this->GetMouseX());
            else debugLines.erase(dupMouseX);
        }
    }

public:
    Raytracer()
    {
        this->sAppName = "Raytracing test";
    }
    virtual ~Raytracer() {}

    bool OnUserCreate() override
    {
        this->distPlane = (this->ScreenWidth() / 2.0) / tan(fov / 2.0);
        this->rayAngleDelta = fov / static_cast<double>(this->ScreenWidth());
        this->player.angle = radians(15.0);
        this->player.x = 292.0;
        this->player.y = 360.0;
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        this->render(fElapsedTime);
        this->handleInput(fElapsedTime);
        return true;
    }

    bool OnUserDestroy() override
    {
        return true;
    }
};



int main()
{
    Raytracer demo;
    if (demo.Construct(320, 200, 1, 1))
        demo.Start();
    return 0;
}
