#include "AABB.h"
#include <algorithm>

glm::vec3 minVec3(glm::vec3 a, glm::vec3 b)
{
    return glm::vec3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

glm::vec3 maxVec3(glm::vec3 a, glm::vec3 b)
{
    return glm::vec3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

AABB::AABB() :
    min(glm::vec3(std::numeric_limits<float>::max())),
    max(glm::vec3(-std::numeric_limits<float>::max()))
{
    extent = max - min;
}

AABB::AABB(const glm::vec3& min, const glm::vec3& max)
    : min(min), max(max)
{
    extent = max - min;
}

AABB::AABB(const glm::vec3& p)
    : min(p), max(p)
{
    extent = max - min;
}

void AABB::expandToInclude(const glm::vec3& p)
{
    min = minVec3(min, p);
    max = maxVec3(max, p);
    extent = max - min;
}

void AABB::expandToInclude(const AABB& b)
{
    min = minVec3(min, b.min);
    max = maxVec3(max, b.max);
    extent = max - min;
}

uint32_t AABB::maxDimension() const
{
    uint32_t result = 0;
    if (extent.y > extent.x)
    {
        result = 1;
        if (extent.z > extent.y) result = 2;
    }
    else if (extent.z > extent.x) result = 2;

    return result;
}

float AABB::surfaceArea() const
{
    return 2.f * (extent.x * extent.z + extent.x * extent.y + extent.y * extent.z);
}
