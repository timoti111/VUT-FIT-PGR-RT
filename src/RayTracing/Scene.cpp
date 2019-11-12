#include "Scene.h"
#include <random>
#include <glm/gtx/norm.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include "Utils/tiny_obj_loader.h"

Scene::Scene() : BVH(1)
{}

void Scene::shapeFromObj(std::string path, std::string name)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;


    std::string warn;
    std::string err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
    {
        throw std::runtime_error(err);
    }

    Shape shapeI;
    shapeI.name = name;
    for (auto& shape : shapes)
    {
        if (shape.name.empty())
            throw std::runtime_error("Unnamed object in file: " + path);

        shapeI.shapeParts.push_back(Scene::ShapePart{ shape.name, (int)primitives.size(), (int)shape.mesh.indices.size() / 3 });
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
        {
            auto t = TrianglePrimitive(this->triangles.size());
            primitives.push_back(t);
            triangles.push_back(Scene::Triangle{
                glm::ivec4(glm::ivec3(shape.mesh.indices[i].vertex_index, shape.mesh.indices[i + 1].vertex_index, shape.mesh.indices[i + 2].vertex_index) + (int)this->vertices.size(), 0),
                glm::ivec4(glm::ivec3(shape.mesh.indices[i].normal_index, shape.mesh.indices[i + 1].normal_index, shape.mesh.indices[i + 2].normal_index) + (int)this->normals.size(), 0),
                glm::ivec4(glm::ivec3(shape.mesh.indices[i].texcoord_index, shape.mesh.indices[i + 1].texcoord_index, shape.mesh.indices[i + 2].texcoord_index) + (int)this->coords.size(), 0)
                                }
            );
        }
    }

    this->shapes.push_back(shapeI);

    for (size_t i = 0; i < attrib.vertices.size(); i += 3)
    {
        this->vertices.push_back(glm::vec4(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2], 1.0f));
    }

    for (size_t i = 0; i < attrib.normals.size(); i += 3)
    {
        this->normals.push_back(glm::vec4(attrib.normals[i], attrib.normals[i + 1], attrib.normals[i + 2], 0.0f));
    }

    for (size_t i = 0; i < attrib.texcoords.size(); i += 2)
    {
        this->coords.push_back(glm::vec2(attrib.texcoords[i], attrib.texcoords[i + 1]));
    }
}

void Scene::instantiateShape(std::string shapeName, glm::mat4x4 objectToWorld)
{
    for (auto& shape : shapes)
    {
        if (shape.name == shapeName)
        {
            for (auto& shapePart : shape.shapeParts)
            {
                Mesh mesh = Mesh(
                    *this,
                    shapePart.name,
                    shapePart.primitiveOffset,
                    shapePart.primitiveCount,
                    objectToWorld);
                meshes.push_back(mesh);
            }
            break;
        }
    }
}

void Scene::removeMesh(std::string meshName)
{
    Mesh* foundMesh = nullptr;
    for (auto& mesh : meshes)
    {
        if (mesh.name == meshName)
        {
            foundMesh = &mesh;
            break;
        }
    }
    if (foundMesh == nullptr)
        throw std::runtime_error("Mesh of name '" + meshName + "' not found");
    updateTree = true;
}

void Scene::setUpdateSceneBVH()
{
    updateSceneBVH = true;
}

void Scene::updateBVHs()
{
    if (!updateSceneBVH)
        return;

    clearPrimitives();
    meshBVHs.clear();
    primitivesGPU.clear();
    for (auto& mesh : meshes)
    {
        addPrimitive(&mesh);
        mesh.bvhOffset = meshBVHs.size();
        mesh.gpuPrimitiveOffset = primitivesGPU.size();
        auto& meshBVH = mesh.getFlatTree();
        meshBVHs.insert(meshBVHs.end(), meshBVH.begin(), meshBVH.end());
        for (auto& treePrims : mesh.getPrimitives())
            primitivesGPU.push_back(*dynamic_cast<GeomPrimitive*>(treePrims));
    }
    update(*this);

    meshesGPU.clear();
    for (auto& treePrimitive : getPrimitives())
        meshesGPU.push_back(*dynamic_cast<Mesh*>(treePrimitive));

    sceneUpdatedCallback();
}

void Scene::sceneUpdatedCallback()
{}

Scene::GeomPrimitive::GeomPrimitive(GLint type, GLint index)
    : TreeObject(), type(type), index(index)
{}

AABB Scene::GeomPrimitive::getAABB(Scene& scene, glm::mat4x4& objectToWorld)
{
    AABB ret;
    switch (type)
    {
        case TRIANGLE:
            Triangle triangle = scene.triangles[index];
            ret.expandToInclude(glm::vec3(objectToWorld * scene.vertices[triangle.vertices.x]));
            ret.expandToInclude(glm::vec3(objectToWorld * scene.vertices[triangle.vertices.y]));
            ret.expandToInclude(glm::vec3(objectToWorld * scene.vertices[triangle.vertices.z]));
            break;
        default:
            break;
    }
    return ret;
}

glm::vec3 Scene::GeomPrimitive::getCentroid(Scene& scene, glm::mat4x4& objectToWorld)
{
    glm::vec3 ret;
    switch (type)
    {
        case TRIANGLE:
            Triangle triangle = scene.triangles[index];
            ret = glm::vec3(objectToWorld * scene.vertices[triangle.vertices.x]);
            ret += glm::vec3(objectToWorld * scene.vertices[triangle.vertices.y]);
            ret += glm::vec3(objectToWorld * scene.vertices[triangle.vertices.z]);
            break;
        default:
            break;
    }
    return ret / 3.0f;
}

Scene::TrianglePrimitive::TrianglePrimitive(int index)
    : GeomPrimitive(TRIANGLE, index)
{}

Scene::Mesh::Mesh(Scene& scene, std::string name, int primitiveOffset, int primitiveCount, glm::mat4x4 objectToWorld) :
    GeomPrimitive(MESH, -1),
    BVH(1),
    scene(&scene),
    name(name),
    objectToWorld(objectToWorld)
{
    for (int i = primitiveOffset; i < primitiveOffset + primitiveCount; i++)
        addPrimitive(&this->scene->primitives[i]);
    updateBVHs();
}

AABB Scene::Mesh::getAABB(Scene& scene, glm::mat4x4& objectToWorld)
{
    return AABB(getFlatTree()[0].min, getFlatTree()[0].max);
}

glm::vec3 Scene::Mesh::getCentroid(Scene& scene, glm::mat4x4& objectToWorld)
{
    AABB own = getAABB(scene, objectToWorld);
    return glm::vec3(
        own.min.x + own.extent.x * 0.5f,
        own.min.y + own.extent.y * 0.5f,
        own.min.z + own.extent.z * 0.5f
    );
}

void Scene::Mesh::setObjectToWorld(glm::mat4x4& objectToWorld)
{
    this->objectToWorld = objectToWorld;
    updateBVHs();
}

glm::mat4x4 Scene::Mesh::getObjectToWorld()
{
    return objectToWorld;
}

void Scene::Mesh::updateBVHs()
{
    update(*this->scene, objectToWorld);
    scene->setUpdateSceneBVH();
}

Scene::GeomPrimitiveGPU::GeomPrimitiveGPU(GeomPrimitive p) :
    type(p.type),
    index(p.index)
{}

Scene::MeshGPU::MeshGPU(Mesh& o) :
    objectToWorld(o.objectToWorld),
    primitiveOffset(o.gpuPrimitiveOffset),
    bvhOffset(o.bvhOffset),
    smoothing(false),
    materialID(0)
{}
