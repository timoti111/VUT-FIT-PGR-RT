#pragma once
#include "Primitive.h"

namespace Geometry
{
    struct Triangle : public Primitive
    {
        Triangle(int index);
    };

    namespace GPU
    {
        struct alignas(16) Triangle
        {
            glm::ivec4 vertices;
            glm::ivec4 normals;
            glm::ivec4 uvs;
        };
    }
}