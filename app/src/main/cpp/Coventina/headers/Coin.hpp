#pragma once

#include "MeshItem.hpp"

namespace game
{
    class Coin : public MeshItem
    {
    public:
        static float yAngle;
        static float yTrans;
        static float yDir;

        static void genGraphics();
        void draw();
        static void update(float dt);
    };
}

