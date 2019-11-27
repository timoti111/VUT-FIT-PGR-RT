#pragma once
#include "Primitive.h"
#include "bvh/BVH.h"

namespace Geometry
{
    class Mesh;
    class Shape;
    struct MeshInstance : public Primitive, public BVH
    {
        MeshInstance(
            Mesh* parent,
            glm::mat4x4 objectToWorld = glm::mat4x4(1.0f)
        );
        AABB getAABB();
        glm::vec3 getCentroid();
        void setObjectToWorld(glm::mat4x4& objectToWorld);
        glm::mat4x4 getObjectToWorld();
        void updateBVHs();
        bool intersect(Ray& ray, Scene& scene, bool occlusion);

        Mesh* parent;
        std::vector<Geometry::Primitive> primitives;
        glm::mat4x4 objectToWorld;
        int gpuPrimitiveOffset;
        int bvhOffset;
        int materialID;
        bool smoothing;
    };

    namespace GPU
    {
        struct alignas(16) MeshInstance
        {
            glm::mat4x4 objectToWorld;
            int primitiveOffset; 
            int bvhOffset;
            int materialID;
            bool smoothing;

            MeshInstance(::Geometry::MeshInstance & o);
            //int getOriginalIndex(Scene & scene);
        };
    }
}