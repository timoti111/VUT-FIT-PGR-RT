#pragma once
#include "bvh/TreeObject.h"
#include <geGL/geGL.h>

namespace Geometry
{
    enum PrimitiveTypes
    {
        MESH,
        TRIANGLE,
        SPHERE,
        CYLINDER
    };

    struct Primitive : public TreeObject
    {
        Primitive(GLint type, GLint index);
        AABB getAABB(Scene& scene, glm::mat4x4& objectToWorld);
        glm::vec3 getCentroid(Scene& scene, glm::mat4x4& objectToWorld);

        GLint type;
        GLint index;
    };

    namespace GPU
    {
        class Mesh;
        struct alignas(4) Primitive
        {
            GLint type;
            GLint index;

            Primitive(::Geometry::Primitive p);
            bool intersect(Ray & ray, ::Geometry::GPU::Mesh & mesh, Scene & scene);
        };
    }
}
