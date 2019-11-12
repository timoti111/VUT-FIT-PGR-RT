#pragma once
#include "AABB.h"

class Scene;

struct TreeObject
{
    //! Return a bounding box for this object
    virtual AABB getAABB(Scene& scene, glm::mat4x4& objectToWorld) = 0;

    //! Return the centroid for this object. (Used in BVH Sorting)
    virtual glm::vec3 getCentroid(Scene& scene, glm::mat4x4& objectToWorld) = 0;
};
