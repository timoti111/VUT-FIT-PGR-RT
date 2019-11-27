#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Geometry
{
    struct Cylinder
    {
        Cylinder(glm::vec3 start, glm::vec3 end, float radius);
        glm::vec4 start;
        glm::vec4 end;
        float radius;
    };

    namespace GPU
    {
        struct alignas(16) Cylinder
        {
            Cylinder(::Geometry::Cylinder cylinder);
            glm::vec4 start;
            glm::vec4 end;
            float radius;
        };
    }
}
