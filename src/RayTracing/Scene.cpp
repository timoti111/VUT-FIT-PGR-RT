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
            auto t = Geometry::Triangle(this->triangles.size());
            primitives.push_back(t);
            triangles.push_back(Geometry::GPU::Triangle{
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

void Scene::instantiateShape(std::string shapeName, glm::mat4x4 objectToWorld, bool asOneMesh)
{
    int primitiveCount = 0;
    for (auto& shape : shapes)
    {
        if (shape.name == shapeName)
        {
            if (asOneMesh)
            {
                for (auto& shapePart : shape.shapeParts)
                {
                    primitiveCount += shapePart.primitiveCount;
                }
                Geometry::Mesh mesh = Geometry::Mesh(
                    *this,
                    shape.name,
                    shape.shapeParts.front().primitiveOffset,
                    primitiveCount,
                    objectToWorld);
                meshes.push_back(mesh);
            }
            else
            {
                for (auto& shapePart : shape.shapeParts)
                {
                    Geometry::Mesh mesh = Geometry::Mesh(
                        *this,
                        shapePart.name,
                        shapePart.primitiveOffset,
                        shapePart.primitiveCount,
                        objectToWorld);
                    meshes.push_back(mesh);
                }
            }
            break;
        }
    }
}

void Scene::removeMesh(std::string meshName)
{
    Geometry::Mesh* foundMesh = nullptr;
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
            primitivesGPU.push_back(*dynamic_cast<Geometry::Primitive*>(treePrims));
    }
    update(*this);

    meshesGPU.clear();
    for (auto& treePrimitive : getPrimitives())
        meshesGPU.push_back(*dynamic_cast<Geometry::Mesh*>(treePrimitive));

    sceneUpdatedCallback();
}

void Scene::sceneUpdatedCallback()
{}
