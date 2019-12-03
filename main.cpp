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

        Vec2d() : x(0), y(0) {}
        Vec2d(int32_t _x, int32_t _y) : x(_x), y(_y) {}

    bool operator==(const Vec2d& other)
    {
        return this->x == other.x && this->y == other.y;
    }
};

class Player
{
public:
    double angle;
    double x;
    double y;
};

enum WALLSIDE { LEFT, RIGHT };

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
        return coords.x/gridSize < 10 && coords.y/gridSize < 10 && coords.x/gridSize >= 0 && coords.y/gridSize >= 0
                && map[coords.x/gridSize][coords.y/gridSize] != 0;
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

    bool isNearZeroAngle(double angle)
    {
        const double eps = 0.001;
        auto isNear = [eps, angle](double near)
        {
            return fabs(angle - near) < eps;
        };
        return isNear(0) ||
            isNear(M_PI_2) ||
            isNear(M_PI) ||
            isNear(M_PI+M_PI_2) ||
            isNear(M_PI+M_PI);
    }

    inline bool isOutOfBounds(Vec2d pos)
    {
        return pos.x < 0 ||
               pos.y < 0 ||
               pos.x >= 10*gridSize ||
               pos.y >= 10*gridSize;
    }

    Vec2d castRayH(double angle, double x, double y)
    {
        Vec2d rayIntersect;
        Vec2d delta;

        // horizontal gridlines
        if (angle < radians(180.0))
            rayIntersect.y = (static_cast<int32_t>(y) / gridSize) * gridSize - 1;
        else
            rayIntersect.y = (static_cast<int32_t>(y) / gridSize) * gridSize + gridSize;
        rayIntersect.x = static_cast<int32_t>(x + static_cast<double>(y - rayIntersect.y) / tan(angle));

        if (angle < radians(180.0))
            delta.y = -gridSize;
        else delta.y = gridSize;

        if (angle >= radians(90.0) && angle < radians(270.0))
            delta.x = -abs(static_cast<int32_t>(static_cast<double>(gridSize) / tan(angle)));
        else delta.x = abs(static_cast<int32_t>(static_cast<double>(gridSize) / tan(angle)));

        while (!hasWall(rayIntersect) && !isOutOfBounds(rayIntersect))
        {
            rayIntersect.x += delta.x;
            rayIntersect.y += delta.y;
        }

        return rayIntersect;
    }

    Vec2d castRayV(double angle, double x, double y)
    {
        Vec2d rayIntersect;
        Vec2d delta;
        // vertical gridlines
        if (angle >= radians(90.0) && angle < radians(270.0))
            rayIntersect.x = (static_cast<int32_t>(x)/gridSize) * gridSize - 1;
        else rayIntersect.x = (static_cast<int32_t>(x)/gridSize) * gridSize + gridSize;
        rayIntersect.y = static_cast<int32_t>(y + static_cast<double>(x - rayIntersect.x) * tan(angle));

        if (angle >= radians(90.0) && angle < radians(270.0))
            delta.x = -gridSize;
        else delta.x = gridSize;

        if (angle < radians(180.0))
            delta.y = -abs(static_cast<int32_t>(gridSize * tan(angle)));
        else delta.y = abs(static_cast<int32_t>(gridSize * tan(angle)));

        while (!hasWall(rayIntersect) && !isOutOfBounds(rayIntersect))
        {
            rayIntersect.x += delta.x;
            rayIntersect.y += delta.y;
        }

        return rayIntersect;
    }

    std::pair<double, WALLSIDE> castRay(double angle, double x, double y, double rayDelta)
    {
        Vec2d IntersectH = castRayH(angle, x, y);
        Vec2d IntersectV = castRayV(angle, x, y);
        double distanceH = dist(Vec2d(x, y), IntersectH);
        double distanceV = dist(Vec2d(x, y), IntersectV);
        double distance;
        WALLSIDE wallSide;

        if (IntersectH == IntersectV)
        {
            distance = dist(Vec2d(x, y), IntersectH);
            wallSide = this->castRay(angle - rayDelta, x, y, rayDelta).second; // cast ray a bit to the left to determine the correct color
            //rayAngleZeroFix = -rayAngleDelta;
        }
        else if ((distanceH > distanceV && !isOutOfBounds(IntersectV)) || isOutOfBounds(IntersectH))
        {
            distance = distanceV;
            wallSide = WALLSIDE::LEFT;
        }
        else if (!isOutOfBounds(IntersectH))
        {
            distance = distanceH;
            wallSide = WALLSIDE::RIGHT;
        }
        else
            distance = std::numeric_limits<double>::infinity();
        return std::pair<double, WALLSIDE>(distance, wallSide);
    }

    olc::Sprite renderScene(float elapsedTime)
    {
        olc::Sprite scene(this->ScreenWidth(), this->ScreenHeight());

        this->SetDrawTarget(&scene);
        this->Clear(olc::BLANK);

        double rayAngle = player.angle - (fov / 2.0);
        for (uint16_t col = 0; col < this->ScreenWidth(); ++col)
        {
            double rayAngleZeroFix = 0.0;
            rayAngle = this->wrapAngle(rayAngle);
            if (isNearZeroAngle(rayAngle))
            {
                rayAngleZeroFix = -rayAngleDelta;
            }

            std::pair<double, WALLSIDE> ray = castRay(rayAngle, player.x, player.y, rayAngleDelta);
            double distance = ray.first;
            WALLSIDE wallSide = ray.second;
            olc::Pixel wallColor = wallSide == WALLSIDE::LEFT ? olc::GREEN : olc::DARK_GREEN;

            if (distance != std::numeric_limits<double>::infinity())
            {
                double sliceHeight = (static_cast<double>(gridSize)/(distance*cos(wrapAngle(rayAngle + rayAngleZeroFix - player.angle)))) * this->distPlane;
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
        const double turnSensitivity = 1.5;
        const double walkSensitivity = 120.0;
        if (this->GetKey(olc::Key::A).bHeld)
            player.angle -= turnSensitivity * elapsedTime;
        if (this->GetKey(olc::Key::D).bHeld)
            player.angle += turnSensitivity * elapsedTime;
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
        this->player.angle = radians(0.378);
        this->player.x = 412.44;
        this->player.y = 393.67;
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
