#include "Cylinder.h"

Geometry::Cylinder::Cylinder(glm::vec3 start, glm::vec3 end, float radius) :
    start(glm::vec4(start, 1.0f)),
    end(glm::vec4(end, 1.0f)),
    radius(radius)
{}

Geometry::GPU::Cylinder::Cylinder(::Geometry::Cylinder cylinder) :
    start(cylinder.start),
    end(cylinder.end),
    radius(cylinder.radius)
{}
