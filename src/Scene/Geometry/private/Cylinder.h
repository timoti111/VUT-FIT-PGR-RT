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
            Cylinder(glm::vec3 start, glm::vec3 end, float radius);
            glm::vec4 start;
            glm::vec4 end;
            float radius;
        };
    }
}
