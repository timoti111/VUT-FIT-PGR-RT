#include "Primitive.h"
#include "Scene/Scene.h"
#include "Triangle.h"
#include "Sphere.h"
#include "Cylinder.h"
#include "Mesh.h"
#include "Scene/Geometry/Shape.h"

Geometry::Primitive::Primitive(GLint type, GLint index, Geometry::MeshInstance* parent)
    : TreeObject(), type(type), index(index), parent(parent)
{}

AABB Geometry::Primitive::getAABB()
{
    AABB ret;
    switch (type)
    {
        case TRIANGLE:
        {
            auto& triangle = parent->parent->triangles[index];
            auto& objectToWorld = parent->objectToWorld;
            auto& vertices = parent->parent->parent->attributes.vertices;
            ret.expandToInclude(glm::vec3(objectToWorld * vertices[triangle.vertices.x]));
            ret.expandToInclude(glm::vec3(objectToWorld * vertices[triangle.vertices.y]));
            ret.expandToInclude(glm::vec3(objectToWorld * vertices[triangle.vertices.z]));
            break;
        }
        case SPHERE:
        {
            auto& sphere = parent->parent->spheres[index];
            ret.expandToInclude(AABB(
                glm::vec3(sphere.sphere) - glm::vec3(sphere.sphere.w),
                glm::vec3(sphere.sphere) + glm::vec3(sphere.sphere.w)
            ));
            break;
        }
        case CYLINDER:
        {
            auto& cylinder = parent->parent->cylinders[index];
            //glm::vec3 cylinderDirection = cylinder.end - cylinder.start;
            //glm::vec3 yzVector = glm::normalize(glm::cross(cylinderDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
            //glm::vec3 xzVector = glm::normalize(glm::cross(cylinderDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
            //glm::vec3 xyVector = glm::normalize(glm::cross(cylinderDirection, glm::vec3(0.0f, 0.0f, 1.0f)));
            //yzVector *= cylinder.radius;
            //xzVector *= cylinder.radius;
            //xyVector *= cylinder.radius;
            //ret.expandToInclude(glm::vec3(cylinder.start) + yzVector);
            //ret.expandToInclude(glm::vec3(cylinder.start) + xzVector);
            //ret.expandToInclude(glm::vec3(cylinder.start) + xyVector);
            //ret.expandToInclude(glm::vec3(cylinder.end) + yzVector);
            //ret.expandToInclude(glm::vec3(cylinder.end) + xzVector);
            //ret.expandToInclude(glm::vec3(cylinder.end) + xyVector);
            //ret.expandToInclude(glm::vec3(cylinder.start) - yzVector);
            //ret.expandToInclude(glm::vec3(cylinder.start) - xzVector);
            //ret.expandToInclude(glm::vec3(cylinder.start) - xyVector);
            //ret.expandToInclude(glm::vec3(cylinder.end) - yzVector);
            //ret.expandToInclude(glm::vec3(cylinder.end) - xzVector);
            //ret.expandToInclude(glm::vec3(cylinder.end) - xyVector);
            AABB bboxStart = AABB(
                glm::vec3(cylinder.start) - glm::vec3(cylinder.radius),
                glm::vec3(cylinder.start) + glm::vec3(cylinder.radius)
            );
            AABB bboxEnd = AABB(
                glm::vec3(cylinder.end) - glm::vec3(cylinder.radius),
                glm::vec3(cylinder.end) + glm::vec3(cylinder.radius)
            );
            ret.expandToInclude(bboxStart);
            ret.expandToInclude(bboxEnd);
            break;
        }
        default:
            break;
    }
    return ret;
}

glm::vec3 Geometry::Primitive::getCentroid()
{
    glm::vec3 ret;
    switch (type)
    {
        case TRIANGLE:
        {
            auto& triangle = parent->parent->triangles[index];
            auto& objectToWorld = parent->objectToWorld;
            auto& vertices = parent->parent->parent->attributes.vertices;
            ret = glm::vec3(objectToWorld * vertices[triangle.vertices.x]);
            ret += glm::vec3(objectToWorld * vertices[triangle.vertices.y]);
            ret += glm::vec3(objectToWorld * vertices[triangle.vertices.z]);
            ret /= 3.0f;
            break;
        }
        case SPHERE:
        {
            auto& sphere = parent->parent->spheres[index];
            ret = glm::vec3(sphere.sphere);
            break;
        }
        case CYLINDER:
        {
            auto& cylinder = parent->parent->cylinders[index];
            ret = glm::vec3(cylinder.start + cylinder.end);
            ret /= 2.0f;
            break;
        }
        default:
        {
            ret = glm::vec3();
            break;
        }
    }
    return ret;
}

const float EPSILON = 1e-8;
bool Geometry::Primitive::intersect(Ray& ray, ::Geometry::MeshInstance& mesh)
{
    switch (type)
    {
        case TRIANGLE:
        {
            auto& triangle = parent->parent->triangles[index];
            auto& objectToWorld = parent->objectToWorld;
            auto& vertices = parent->parent->parent->attributes.vertices;
            glm::vec3 vert0 = objectToWorld * vertices[triangle.vertices.x];
            glm::vec3 vert1 = objectToWorld * vertices[triangle.vertices.y];
            glm::vec3 vert2 = objectToWorld * vertices[triangle.vertices.z];
            glm::vec3 edge1 = vert1 - vert0;
            glm::vec3 edge2 = vert2 - vert0;
            // begin calculating determinant - also used to calculate U parameter
            glm::vec3 pvec = glm::cross(glm::vec3(ray.direction), edge2);
            // if determinant is near zero, ray lies in plane of triangle
            float det = dot(edge1, pvec);
            // use backface culling
            if (det < EPSILON)
                return false;
            float inv_det = 1.0f / det;
            // calculate distance from vert0 to ray origin
            glm::vec3 tvec = glm::vec3(ray.origin) - vert0;
            // calculate U parameter and test bounds
            float u = dot(tvec, pvec) * inv_det;
            if (u < 0.0f || u > 1.0f)
                return false;
            // prepare to test V parameter
            glm::vec3 qvec = cross(tvec, edge1);
            // calculate V parameter and test bounds
            float v = glm::dot(glm::vec3(ray.direction), qvec) * inv_det;
            if (v < 0.0f || u + v > 1.0f)
                return false;
            // calculate t, ray intersects triangle
            float t = dot(edge2, qvec) * inv_det;
            if (t < ray.t)
                ray.t = t;
            return true;
        }
        case SPHERE:
        {
            auto& sphere = parent->parent->spheres[index];
            return false;
        }
        case CYLINDER:
        {
            auto& cylinder = parent->parent->cylinders[index];
            return false;
        }
        default:
            return false;
    }
}

Geometry::GPU::Primitive::Primitive(::Geometry::Primitive p) :
    type(p.type),
    index(p.index)
{}
