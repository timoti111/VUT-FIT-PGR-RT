#pragma once

#include <cstdint>
#include <glm/vec3.hpp>
#include <limits>

struct AABB
{
    glm::vec3 min, max, extent;
    AABB();
    AABB(const glm::vec3& min, const glm::vec3& max);
    AABB(const glm::vec3& p);

    void expandToInclude(const glm::vec3& p);
    void expandToInclude(const AABB& b);
    uint32_t maxDimension() const;
    float surfaceArea() const;
private:
};
