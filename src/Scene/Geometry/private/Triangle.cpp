#include "Triangle.h"
#include <glm/vec3.hpp>
const float EPSILON = 1e-8;

Geometry::Triangle::Triangle(int index) :
    Primitive(TRIANGLE, index)
{}
