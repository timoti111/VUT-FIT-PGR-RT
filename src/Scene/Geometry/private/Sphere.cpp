#include "Sphere.h"

Geometry::Sphere::Sphere(glm::vec3 position, float radius) :
    sphere(glm::vec4(position, radius))
{}

Geometry::GPU::Sphere::Sphere(::Geometry::Sphere sphere) :
    sphere(sphere.sphere)
{}
