#pragma once
#include "bvh/TreeObject.h"
#include <geGL/geGL.h>
#include <memory>
#include "RayTracing/Ray.h"

namespace Geometry
{
    enum PrimitiveTypes
    {
        MESH,
        TRIANGLE,
        SPHERE,
        CYLINDER
    };

    class MeshInstance;
    struct Primitive : public TreeObject
    {
        Primitive(GLint type, GLint index, MeshInstance* parent);
        AABB getAABB();
        glm::vec3 getCentroid();
        bool intersect(Ray& ray, MeshInstance& mesh);

        GLint type;
        GLint index;
        MeshInstance* parent;

    };

    namespace GPU
    {
        struct alignas(4) Primitive
        {
            GLint type;
            GLint index;

            Primitive(::Geometry::Primitive p);
        };
    }
}
