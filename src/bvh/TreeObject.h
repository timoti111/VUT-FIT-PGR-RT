#pragma once
#include "AABB.h"
#include <glm/mat4x4.hpp>

struct TreeObject
{
    //! Return a bounding box for this object
    virtual AABB getAABB() = 0;

    //! Return the centroid for this object. (Used in BVH Sorting)
    virtual glm::vec3 getCentroid() = 0;
};
