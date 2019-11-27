#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Geometry
{
    struct Sphere
    {
        Sphere(glm::vec3 position, float radius);
        glm::vec4 sphere; // xyz - position, w - radius
    };
    namespace GPU
    {
        struct alignas(16) Sphere
        {
            Sphere(::Geometry::Sphere sphere);
            glm::vec4 sphere; // xyz - position, w - radius
        };
    }
}