#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Geometry
{
    struct alignas(16) Triangle
    {
        glm::vec4 vertices[3];
        glm::vec4 normals[3];
        glm::vec2 coords[3];
    };
}