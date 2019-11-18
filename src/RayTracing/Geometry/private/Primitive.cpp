#include "Primitive.h"
#include "RayTracing/Scene.h"
#include "Triangle.h"
#include "Sphere.h"
#include "Cylinder.h"

Geometry::Primitive::Primitive(GLint type, GLint index)
    : TreeObject(), type(type), index(index)
{}

AABB Geometry::Primitive::getAABB(Scene& scene, glm::mat4x4& objectToWorld)
{
    AABB ret;
    switch (type)
    {
        case TRIANGLE:
            Geometry::GPU::Triangle triangle = scene.triangles[index];
            ret.expandToInclude(glm::vec3(objectToWorld * scene.vertices[triangle.vertices.x]));
            ret.expandToInclude(glm::vec3(objectToWorld * scene.vertices[triangle.vertices.y]));
            ret.expandToInclude(glm::vec3(objectToWorld * scene.vertices[triangle.vertices.z]));
            break;
        case SPHERE:
            Geometry::GPU::Sphere sphere = scene.spheres[index];
            ret.expandToInclude(AABB(
                glm::vec3(sphere.sphere) - glm::vec3(sphere.sphere.w),
                glm::vec3(sphere.sphere) + glm::vec3(sphere.sphere.w)
            ));
            break;
        case CYLINDER:
            Geometry::GPU::Cylinder cylinder = scene.cylinders[index];
            glm::vec3 cylinderDirection = cylinder.end - cylinder.start;
            glm::vec3 yzVector = glm::normalize(glm::cross(cylinderDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
            glm::vec3 xzVector = glm::normalize(glm::cross(cylinderDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 xyVector = glm::normalize(glm::cross(cylinderDirection, glm::vec3(0.0f, 0.0f, 1.0f)));
            yzVector *= cylinder.radius;
            xzVector *= cylinder.radius;
            xyVector *= cylinder.radius;
            ret.expandToInclude(glm::vec3(cylinder.start) + yzVector);
            ret.expandToInclude(glm::vec3(cylinder.start) + xzVector);
            ret.expandToInclude(glm::vec3(cylinder.start) + xyVector);
            ret.expandToInclude(glm::vec3(cylinder.end) + yzVector);
            ret.expandToInclude(glm::vec3(cylinder.end) + xzVector);
            ret.expandToInclude(glm::vec3(cylinder.end) + xyVector);
            ret.expandToInclude(glm::vec3(cylinder.start) - yzVector);
            ret.expandToInclude(glm::vec3(cylinder.start) - xzVector);
            ret.expandToInclude(glm::vec3(cylinder.start) - xyVector);
            ret.expandToInclude(glm::vec3(cylinder.end) - yzVector);
            ret.expandToInclude(glm::vec3(cylinder.end) - xzVector);
            ret.expandToInclude(glm::vec3(cylinder.end) - xyVector);
            //AABB bboxStart = AABB(
            //    glm::vec3(cylinder.start) - glm::vec3(cylinder.radius),
            //    glm::vec3(cylinder.start) + glm::vec3(cylinder.radius)
            //);
            //AABB bboxEnd = AABB(
            //    glm::vec3(cylinder.end) - glm::vec3(cylinder.radius),
            //    glm::vec3(cylinder.end) + glm::vec3(cylinder.radius)
            //);
            //ret.expandToInclude(bboxStart);
            //ret.expandToInclude(bboxEnd);
            break;
        default:
            break;
    }
    return ret;
}

glm::vec3 Geometry::Primitive::getCentroid(Scene& scene, glm::mat4x4& objectToWorld)
{
    glm::vec3 ret;
    switch (type)
    {
        case TRIANGLE:
            Geometry::GPU::Triangle triangle = scene.triangles[index];
            ret = glm::vec3(objectToWorld * scene.vertices[triangle.vertices.x]);
            ret += glm::vec3(objectToWorld * scene.vertices[triangle.vertices.y]);
            ret += glm::vec3(objectToWorld * scene.vertices[triangle.vertices.z]);
            ret /= 3.0f;
            break;
        case SPHERE:
            Geometry::GPU::Sphere sphere = scene.spheres[index];
            ret = glm::vec3(sphere.sphere);
            break;
        case CYLINDER:
            Geometry::GPU::Cylinder cylinder = scene.cylinders[index];
            ret = glm::vec3(cylinder.start + cylinder.end);
            ret /= 2.0f;
            break;
        default:
            break;
    }
    return ret;
}

Geometry::GPU::Primitive::Primitive(::Geometry::Primitive p) :
    type(p.type),
    index(p.index)
{}
