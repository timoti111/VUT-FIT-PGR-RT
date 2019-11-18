#pragma once
#include "Primitive.h"

namespace Geometry
{
    struct Sphere : public Primitive
    {
        Sphere(int index);
    };
    namespace GPU
    {
        struct alignas(16) Sphere
        {
            glm::ivec4 sphere; // xyz - position, w - radius
        };
    }
}