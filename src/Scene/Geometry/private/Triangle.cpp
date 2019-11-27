#include "Triangle.h"
#include <glm/vec3.hpp>
const float EPSILON = 1e-8;

Geometry::GPU::Triangle::Triangle(::Geometry::Triangle triangle, glm::ivec3 offset) :
    vertices(triangle.vertices + offset.x),
    normals(triangle.normals + offset.y),
    uvs(triangle.uvs + offset.z)

{}

Geometry::Triangle::Triangle(glm::ivec4 vertices, glm::ivec4 normals, glm::ivec4 uvs) :
    vertices(vertices),
    normals(normals),
    uvs(uvs)
{}
