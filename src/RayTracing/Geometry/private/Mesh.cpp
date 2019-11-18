#include "Mesh.h"
#include "RayTracing/Scene.h"

Geometry::Mesh::Mesh(Scene& scene, std::string name, int primitiveOffset, int primitiveCount, glm::mat4x4 objectToWorld) :
    Primitive(MESH, -1),
    BVH(1),
    scene(&scene),
    name(name),
    objectToWorld(objectToWorld)
{
    for (int i = primitiveOffset; i < primitiveOffset + primitiveCount; i++)
        addPrimitive(&this->scene->primitives[i]);
    updateBVHs();
}

AABB Geometry::Mesh::getAABB(Scene& scene, glm::mat4x4& objectToWorld)
{
    return AABB(getFlatTree()[0].min, getFlatTree()[0].max);
}

glm::vec3 Geometry::Mesh::getCentroid(Scene& scene, glm::mat4x4& objectToWorld)
{
    AABB own = getAABB(scene, objectToWorld);
    return glm::vec3(
        own.min.x + own.extent.x * 0.5f,
        own.min.y + own.extent.y * 0.5f,
        own.min.z + own.extent.z * 0.5f
    );
}

void Geometry::Mesh::setObjectToWorld(glm::mat4x4& objectToWorld)
{
    this->objectToWorld = objectToWorld;
    updateBVHs();
}

glm::mat4x4 Geometry::Mesh::getObjectToWorld()
{
    return objectToWorld;
}

void Geometry::Mesh::updateBVHs()
{
    update(*this->scene, objectToWorld);
    scene->setUpdateSceneBVH();
}

Geometry::GPU::Mesh::Mesh(::Geometry::Mesh& o) :
    objectToWorld(o.objectToWorld),
    primitiveOffset(o.gpuPrimitiveOffset),
    bvhOffset(o.bvhOffset),
    smoothing(false),
    materialID(0)
{}
