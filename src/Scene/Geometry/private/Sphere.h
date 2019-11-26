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
            Sphere(glm::vec3 position, float radius);
            glm::vec4 sphere; // xyz - position, w - radius
        };
    }
}