#pragma once

#include "MeshItem.hpp"
#include "shapes.h"
#include <glm/vec3.hpp>
#include <vector>
#include <cstdint>

namespace game
{
    class Ring : public MeshItem
    {
    public:
        static float yAngle;
        static float yTrans;
        static float yDir;

        static void genGraphics();
        void draw();
        static void update(float dt);

        static std::vector<Shape*> shapes;
    };
}

