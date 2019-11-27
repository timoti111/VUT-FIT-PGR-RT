#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Geometry
{
    struct Triangle
    {
        Triangle(glm::ivec4 vertices, glm::ivec4 normals, glm::ivec4 uvs);
        glm::ivec4 vertices;
        glm::ivec4 normals;
        glm::ivec4 uvs;
    };

    namespace GPU
    {
        struct alignas(16) Triangle
        {
            Triangle(::Geometry::Triangle triangle, glm::ivec3 offset);
            glm::ivec4 vertices;
            glm::ivec4 normals;
            glm::ivec4 uvs;
        };
    }
}