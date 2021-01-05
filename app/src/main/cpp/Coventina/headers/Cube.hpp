#pragma once

#include "MeshItem.hpp"
#include <glm/vec3.hpp>
#include <cstdint>

namespace game
{
    class Cube : public MeshItem
    {
    public:
        static void genGraphics();
        void draw();
    };
}

