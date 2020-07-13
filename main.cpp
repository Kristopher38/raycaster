#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <utility>
#include <vector>
#include <limits>
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

typedef olc::v2d_generic<double> Vec2d;
typedef olc::v2d_generic<int64_t> Vec2i;

class Player
{
    public:
        Vec2d pos;
        Vec2d dir;
        Player() {}
        Player(Vec2d _pos, Vec2d _dir) : pos(_pos), dir(_dir) {}
};

enum WALL_SIDE { NONE, VERTICAL, HORIZONTAL };

class Raytracer : public olc::PixelGameEngine
{
private:
    const double tileSize = 1.0f;
    const double fov = M_PI_2 * 2.0f/3.0f; // 60 degrees
    Player player = Player(Vec2d(5.3, 5.7), Vec2d(-1, 0));

    /*      180deg
     *
     * 90deg       270deg
     *
     *       0deg
     */
    std::array<std::array<uint8_t, 10>, 10> map = {{
        {{1, 1, 1, 1, 1, 0, 1, 1, 1, 1}},
        {{1, 0, 0, 0, 0, 0, 1, 1, 0, 1}},
        {{1, 0, 0, 0, 0, 0, 0, 0, 0, 1}},
        {{1, 0, 0, 0, 0, 0, 0, 0, 1, 1}},
        {{1, 0, 0, 0, 0, 0, 0, 0, 1, 1}},
        {{1, 0, 0, 0, 0, 0, 0, 0, 0, 1}},
        {{1, 0, 0, 0, 0, 0, 0, 0, 0, 1}},
        {{1, 0, 0, 0, 0, 0, 0, 0, 0, 1}},
        {{1, 0, 0, 1, 1, 0, 0, 0, 0, 1}},
        {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}
    }};
    std::vector<int> debugLines;

    inline double dist(Vec2d v1, Vec2d v2)
    {
        return sqrt((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y));
    }

    inline double dist(Vec2d v1, Vec2d v2, double rayAngle)
    {
        return static_cast<double>(abs(v1.x - v2.x)) / cos(rayAngle);
    }

    inline bool hasWall(Vec2i coords)
    {
        return map.at(coords.y).at(coords.x) != 0;
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

    Vec2d rotate(Vec2d vec, double angle)
    {
        Vec2d rotated;
        rotated.x = vec.x * std::cos(angle) - vec.y * std::sin(angle);
        rotated.y = vec.x * std::sin(angle) + vec.y * std::cos(angle);
        return rotated;
    }

    inline Vec2d getCameraPlane(Vec2d dir)
    {
        return dir.perp().norm() * std::tan(this->fov / 2.0f) * dir.mag();
    }

    inline olc::Sprite newLayer(uint32_t w, uint32_t h)
    {
        olc::Sprite sprite(w, h);
        this->SetDrawTarget(&sprite);
        this->Clear(olc::BLANK);
        this->SetDrawTarget(nullptr);
        return sprite;
    }

    inline olc::Sprite newLayer()
    {
        return this->newLayer(this->ScreenWidth(), this->ScreenHeight());
    }

    olc::Sprite renderMap(float elapsedTime, double scale)
    {
        olc::Sprite mapView = this->newLayer();//this->newLayer(map.size() * scale, map[0].size() * scale);
        Vec2d cameraPlane = this->getCameraPlane(player.dir);

        olc::Sprite debug = this->newLayer();
        this->SetDrawTarget(&debug);
        this->DrawString(202, 0, std::to_string(player.pos.x));
        this->DrawString(202, 8, std::to_string(player.pos.y));
        this->DrawString(202, 16, std::to_string(player.dir.x));
        this->DrawString(202, 24, std::to_string(player.dir.y));

        olc::Sprite camera = this->newLayer();
        this->SetDrawTarget(&camera);
        Vec2d curTile;
        curTile.x = std::floor(player.pos.x) - player.pos.x;
        curTile.y = std::floor(player.pos.y) - player.pos.y;
        this->FillRect((player.pos + curTile) * scale + Vec2d(1.0, 1.0), Vec2d(scale, scale) - Vec2d(1.0, 1.0), olc::GREY);
        this->FillCircle(player.pos * scale, 0.25 * scale, olc::RED);
        this->DrawLine(player.pos * scale, (player.pos+player.dir) * scale, olc::YELLOW);
        this->DrawLine((player.pos+player.dir-cameraPlane) * scale,
                       (player.pos+player.dir+cameraPlane) * scale, olc::MAGENTA);
        this->DrawLine(player.pos * scale, (player.pos+player.dir-cameraPlane) * scale, olc::DARK_MAGENTA);
        this->DrawLine(player.pos * scale, (player.pos+player.dir+cameraPlane) * scale, olc::DARK_MAGENTA);

        for (int32_t col = 321; col < 322 /*this->ScreenWidth()*/; ++col)
        {
            Vec2d rayDir = player.dir + cameraPlane * (2 * static_cast<double>(col - 321) / static_cast<double>(this->ScreenWidth() - 321) - 1);
            this->DrawLine(player.pos * scale, (player.pos + rayDir.norm() * (Vec2d(map.size(), map[0].size()) - player.pos).mag()) * scale, olc::BLUE);

            Vec2d rayStepDist;
            // scale normalized rayDir by a factor that first makes x component equal to 1
            rayStepDist.x = (rayDir.norm() * 1 / rayDir.x).mag();
            // ...and second y component equal to 1, and compute their magnitudes
            rayStepDist.y = (rayDir.norm() * 1 / rayDir.y).mag();

            this->SetDrawTarget(&debug);
//            this->DrawString(202, 32, std::to_string(rayStepDist.x));
//            this->DrawString(202, 40, std::to_string(rayStepDist.y));



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
                while (!hasWall(mapPos))
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
                    this->FillRect((mapPos * scale) + Vec2i(1, 1), Vec2d(scale, scale) - Vec2d(1.0, 1.0), olc::Pixel(43, 169, 252)); // light blue
                }
            }
            catch (std::out_of_range& e)
            {
                side = WALL_SIDE::NONE;
            }

            this->DrawString(0, 48, std::to_string(curRayDist.mag()));
            Vec2d hitPos = (player.pos + rayDir.norm()) * curRayDist.mag();
            this->FillCircle(hitPos * scale, 0.1 * scale);

            //Vec2d hitPos = player.pos + curRayDist;
            //double hitDist = dist(player.pos, curRayDist);

            olc::Pixel wallColor;
            double hitDist;
            switch (side)
            {
                case WALL_SIDE::NONE:
                    wallColor = olc::NONE;
                    hitDist = std::numeric_limits<double>::infinity();
                    break;
                case WALL_SIDE::VERTICAL:
                    wallColor = olc::DARK_GREEN;
                    hitDist = hitPos.mag();// (mapPos.x - player.pos.x + (1 - mapStep.x) / 2) / rayDir.x;
                    break;
                case WALL_SIDE::HORIZONTAL:
                    wallColor = olc::GREEN;
                    hitDist = hitPos.mag();//(mapPos.y - player.pos.y + (1 - mapStep.y) / 2) / rayDir.y;
                    break;
            }

            int32_t height = this->ScreenHeight() / hitDist;
            this->SetDrawTarget(&mapView);
            this->DrawLine(col, this->ScreenHeight()/2 - height/2, col, this->ScreenHeight()/2 + height/2, wallColor);
        }
        olc::Sprite airs = this->newLayer();
        olc::Sprite walls = this->newLayer();
        for (int32_t row = 0; row < map.size(); ++row)
        {
            for (int32_t col = 0; col < map[row].size(); ++col)
            {
                if (map[col][row] != 0)
                {
                    this->SetDrawTarget(&walls);
                    this->FillRect(row * scale, col * scale, scale, scale, olc::Pixel(61, 201, 56)); // slightly dark green
                    this->DrawRect(row * scale, col * scale, scale, scale, olc::Pixel(72, 245, 66)); // bright green
                }
                else
                {
                    this->SetDrawTarget(&airs);
                    this->DrawRect(row * scale, col * scale, scale, scale, olc::Pixel(38, 125, 35)); // darkest green
                }
            }
        }
        this->SetDrawTarget(&mapView);
        this->DrawSprite(0, 0, &airs);
        this->DrawSprite(0, 0, &walls);
        this->DrawSprite(0, 0, &debug);
        this->DrawSprite(0, 0, &camera);
        this->SetDrawTarget(nullptr);

        return mapView;
    }

    olc::Sprite renderScene(float elapsedTime)
    {
        olc::Sprite scene = this->newLayer();


        olc::Sprite mapView = renderMap(elapsedTime, 20);
        this->SetDrawTarget(&scene);
        //this->DrawSprite(0, 0, &debug);
        this->DrawSprite(0, 0, &mapView);

        Vec2d cameraPlane = this->getCameraPlane(player.dir);



//        for (int32_t col = 0; col < this->ScreenWidth(); ++col)
//        {
//            // ray direction is the sum of direction vector and plane vector multiplied by a coefficient
//            // in range <-1; 1> calculated from current column to get current point (direction) on the plane vector
//            Vec2d rayDir = player.dir + plane * (2 * col / this->ScreenWidth() - 1);

//            Vec2d rayStepDist;
//            // scale normalized rayDir by a factor that first makes x component equal to 1
//            rayStepDist.x = std::abs(1 / rayDir.x);//(rayDir.norm() * 1 / rayDir.x).mag();
//            // ...and second y component equal to 1, and compute their magnitudes
//            rayStepDist.y = std::abs(1 / rayDir.y);//(rayDir.norm() * 1 / rayDir.y).mag();

//            Vec2d curRayDist;
//            // based on the direction of the ray, calculate initial distances to respective x (vertical) and y (horizontal) grid intersections
//            if (rayDir.x > 0)
//                curRayDist.x = (std::ceil(player.pos.x) - player.pos.x) * rayStepDist.x;
//            else
//                curRayDist.x = (player.pos.x - std::floor(player.pos.x)) * rayStepDist.x;
//            if (rayDir.y > 0)
//                curRayDist.y = (std::ceil(player.pos.y) - player.pos.y) * rayStepDist.y;
//            else
//                curRayDist.y = (player.pos.y - std::floor(player.pos.y)) * rayStepDist.y;

//            Vec2i mapPos;
//            mapPos.x = static_cast<uint32_t>(player.pos.x);
//            mapPos.y = static_cast<uint32_t>(player.pos.y);
//            Vec2i mapStep;
//            mapStep.x = std::signbit(rayDir.x) ? -1 : 1;
//            mapStep.y = std::signbit(rayDir.y) ? -1 : 1;

//            WALL_SIDE side = WALL_SIDE::NONE;
//            while (!hasWall(mapPos))
//            {
//                if (curRayDist.x < curRayDist.y)
//                {
//                    curRayDist.x += rayStepDist.x;
//                    mapPos.x += mapStep.x;
//                    side = WALL_SIDE::VERTICAL;
//                }
//                else
//                {
//                    curRayDist.y += rayStepDist.y;
//                    mapPos.y += mapStep.y;
//                    side = WALL_SIDE::HORIZONTAL;
//                }
//            }

//            //Vec2d hitPos = player.pos + rayDir.norm() * curRayDist.mag();

//            //Vec2d hitPos = player.pos + curRayDist;
//            //double hitDist = dist(player.pos, curRayDist);

//            olc::Pixel wallColor;
//            double hitDist;
//            switch (side)
//            {
//                case WALL_SIDE::NONE:
//                    wallColor = olc::NONE;
//                    hitDist = 0.0f;
//                    break;
//                case WALL_SIDE::VERTICAL:
//                    wallColor = olc::DARK_GREEN;
//                    hitDist = (mapPos.x - player.pos.x + (1 - mapStep.x) / 2) / rayDir.x;
//                    break;
//                case WALL_SIDE::HORIZONTAL:
//                    wallColor = olc::GREEN;
//                    hitDist = (mapPos.y - player.pos.y + (1 - mapStep.y) / 2) / rayDir.y;
//                    break;
//            }

//            int32_t height = this->ScreenHeight() / hitDist;

//            this->DrawLine(col, this->ScreenHeight()/2 - height/2, col, this->ScreenHeight()/2 + height/2, wallColor);
//        }

        this->SetDrawTarget(nullptr);
        return scene;
    }

//    olc::Sprite renderDebug(float elapsedTime)
//    {
//        olc::Sprite debugInfo(this->ScreenWidth(), this->ScreenHeight());
//        this->SetDrawTarget(&debugInfo);
//        this->Clear(olc::BLANK);
//        uint16_t textCounter = 0;

//        this->DrawString(0, 8*textCounter++, std::to_string(1.0f/elapsedTime));
//        this->DrawString(0, 8*textCounter++, std::to_string(player.angle * 180.0/M_PI), olc::RED);
//        this->DrawString(0, 8*textCounter++, std::to_string(player.x), olc::RED);
//        this->DrawString(0, 8*textCounter++, std::to_string(player.y), olc::RED);

//        for (std::vector<int>::iterator it = debugLines.begin(); it != debugLines.end(); ++it)
//        {
//            this->DrawLine(*it, 0, *it, this->ScreenHeight(), olc::DARK_RED);
//            this->DrawString(0, 8*textCounter++, std::to_string(*it), olc::RED);
//        }

//        this->SetDrawTarget(nullptr);
//        return debugInfo;
//    }

    void render(float elapsedTime)
    {
        this->SetPixelMode(olc::Pixel::Mode::MASK);
        this->Clear(olc::BLACK);

        olc::Sprite scene = this->renderScene(elapsedTime);
        this->DrawSprite(0, 0, &scene);
//        olc::Sprite debugInfo = this->renderDebug(elapsedTime);
//        this->DrawSprite(0, 0, &debugInfo);
    }



    void handleInput(float elapsedTime)
    {
        const double turnSensitivity = 2.0;
        const double walkSensitivity = 2.5;
        if (this->GetKey(olc::Key::A).bHeld)
        {
            player.dir = rotate(player.dir, -turnSensitivity * elapsedTime);
            //plane = rotate(plane, -turnSensitivity * elapsedTime);
        }
        if (this->GetKey(olc::Key::D).bHeld)
        {
            player.dir = rotate(player.dir, turnSensitivity * elapsedTime);
            //plane = rotate(plane, turnSensitivity * elapsedTime);
        }

        if (this->GetKey(olc::Key::W).bHeld)
            player.pos += player.dir * walkSensitivity * elapsedTime;
        if (this->GetKey(olc::Key::S).bHeld)
            player.pos -= player.dir * walkSensitivity * elapsedTime;

//        if (this->GetMouse(0).bPressed)
//        {
//            int mouseX = this->GetMouseX();
//            std::vector<int>::iterator dupMouseX = std::find(debugLines.begin(), debugLines.end(), mouseX);
//            if (dupMouseX == debugLines.end())
//                debugLines.push_back(this->GetMouseX());
//            else debugLines.erase(dupMouseX);
//        }
    }

public:
    Raytracer()
    {
        this->sAppName = "Raytracing test";
    }
    virtual ~Raytracer() {}

    bool OnUserCreate() override
    {
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
    if (demo.Construct(640, 400, 2, 2))
        demo.Start();
    return 0;
}
