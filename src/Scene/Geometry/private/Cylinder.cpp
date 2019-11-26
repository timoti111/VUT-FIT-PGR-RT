#include "Cylinder.h"

Geometry::Cylinder::Cylinder(int index) :
    Primitive(CYLINDER, index)
{}

Geometry::GPU::Cylinder::Cylinder(glm::vec3 start, glm::vec3 end, float radius)
{
    this->start = glm::vec4(start, 1.0f);
    this->end = glm::vec4(end, 1.0f);
    this->radius = radius;
}
