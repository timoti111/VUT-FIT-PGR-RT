#pragma once
#include "Primitive.h"

namespace Geometry
{
    struct Cylinder : public Primitive
    {
        Cylinder(int index);
    };

    namespace GPU
    {
        struct alignas(16) Cylinder
        {
            glm::ivec4 start;
            glm::ivec4 end;
            float radius;
        };
    }
}
