#pragma once
#include "Primitive.h"
#include "bvh/BVH.h"

class Scene;
namespace Geometry
{
    struct Mesh : public Primitive, public BVH
    {
        Mesh(
            Scene& scene,
            std::string name,
            int primitiveOffset,
            int primitiveCount,
            glm::mat4x4 objectToWorld = glm::mat4x4(1.0f)
        );
        AABB getAABB(Scene& scene, glm::mat4x4& objectToWorld);
        glm::vec3 getCentroid(Scene& scene, glm::mat4x4& objectToWorld);
        void setObjectToWorld(glm::mat4x4& objectToWorld);
        glm::mat4x4 getObjectToWorld();
        void updateBVHs();

        Scene* scene;
        std::string name;
        glm::mat4x4 objectToWorld;
        int gpuPrimitiveOffset;
        int bvhOffset;
        int materialID;
        bool selected;
        bool smoothing;
    };

    namespace GPU
    {
        struct alignas(16) Mesh
        {
            glm::mat4x4 objectToWorld;
            int primitiveOffset; 
            int bvhOffset;
            int materialID;
            bool smoothing;

            Mesh(::Geometry::Mesh & o);
            bool intersect(Ray & ray, Scene & scene, bool occlusion);
            int getOriginalIndex(Scene & scene);
        };
    }
}