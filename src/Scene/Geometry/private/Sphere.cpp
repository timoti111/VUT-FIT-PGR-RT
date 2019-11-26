#include "Sphere.h"

Geometry::Sphere::Sphere(int index) :
    Primitive(SPHERE, index)
{}

Geometry::GPU::Sphere::Sphere(glm::vec3 position, float radius)
{
    this->sphere = glm::vec4(position.x, position.y, position.z, radius);
}
