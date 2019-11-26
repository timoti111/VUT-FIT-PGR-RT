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
                shape.references.push_back(meshes.size());
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
                    shape.references.push_back(meshes.size());
                    meshes.push_back(mesh);
                }
            }
            break;
        }
    }
}

void Scene::removeShape(std::string meshName)
{
    Shape* foundShape = nullptr;
    for (int i = 0; i < shapes.size(); i++)
    {
        if (shapes[i].name == meshName)
        {
            foundShape = &shapes[i];
            if (!shapes[i].references.empty())
            {
                //int change = 1;
                //int nextIndex = shape.references[0];
                for (int meshIndex : shapes[i].references)
                {
                    //change = 
                    //nextIndex = nextIndex + change - 1;;
                    meshes.erase(meshes.begin() + meshIndex); // TODO REMOVING CORRECTION
                }
            }
            shapes.erase(shapes.begin() + i);
           /* primitives.erase(primitives.begin() + foundMesh->gpuPrimitiveOffset, primitives.begin() + foundMesh->gpuPrimitiveOffset + foundMesh->)
            foundMesh.primitive*/
            break;
        }
    }
    if (foundShape == nullptr)
        throw std::runtime_error("Shape of name '" + meshName + "' not found");
    updateSceneBVH = true;
}

void Scene::selectMesh(int index)
{
    if (actualSelectedMesh != -1 || index == -1)
    {
        try
        {
            removeShape("SelectedObject");
        }
        catch (const std::runtime_error& e)
        {
        }
    }

    if (index != -1)
    {
        this->shapes.push_back(createSelectedObjectShape(index));
        instantiateShape("SelectedObject", glm::mat4x4(1.0f), true);
        //Geometry::Mesh mesh = Geometry::Mesh(
        //    *this,
        //    "SelectedObject",
        //    actualPrimitiveCount,
        //    20,
        //    glm::mat4x4(1.0f));
        //meshes.push_back(mesh);

    }
    // createMesh "SelectedObject"

    actualSelectedMesh = index;
    updateBVHs();
}

void Scene::setUpdateSceneBVH()
{
    updateSceneBVH = true;
}

void Scene::updateBVHs()
{
    if (!updateSceneBVH)
        return;
    updateSceneBVH = false;

    clearPrimitives();
    meshBVHs.clear();
    primitivesGPU.clear();
    Geometry::Mesh* selectedMesh = nullptr;
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

Scene::Shape Scene::createSelectedObjectShape(int index)
{
    float radius = 0.01f;
    Geometry::Mesh mesh = meshes[index];
    AABB meshAABB = mesh.getAABB(*this, mesh.objectToWorld);
    std::vector<glm::vec3> p;
    p.push_back(glm::vec3(meshAABB.min));
    p.push_back(glm::vec3(meshAABB.max.x, meshAABB.min.y, meshAABB.min.z));
    p.push_back(glm::vec3(meshAABB.max.x, meshAABB.min.y, meshAABB.max.z));
    p.push_back(glm::vec3(meshAABB.min.x, meshAABB.min.y, meshAABB.max.z));
    p.push_back(glm::vec3(meshAABB.min.x, meshAABB.max.y, meshAABB.min.z));
    p.push_back(glm::vec3(meshAABB.max.x, meshAABB.max.y, meshAABB.min.z));
    p.push_back(glm::vec3(meshAABB.max.x, meshAABB.max.y, meshAABB.max.z));
    p.push_back(glm::vec3(meshAABB.min.x, meshAABB.max.y, meshAABB.max.z));
    glm::vec3 lastBotPoint = p.at(3);
    glm::vec3 lastTopPoint = p.at(7);

    ShapePart part = ShapePart{ "bbox", static_cast<int>(primitives.size()), 20 };
    for (int i = 0; i < 4; i++)
    {
        auto t = Geometry::Cylinder(this->cylinders.size());
        primitives.push_back(t);
        auto cylinderGPU = Geometry::GPU::Cylinder(p.at(i), lastBotPoint, radius);
        cylinders.push_back(cylinderGPU);

        t = Geometry::Cylinder(this->cylinders.size());
        primitives.push_back(t);
        cylinderGPU = Geometry::GPU::Cylinder(p.at(i + 4), lastTopPoint, radius);
        cylinders.push_back(cylinderGPU);

        t = Geometry::Cylinder(this->cylinders.size());
        primitives.push_back(t);
        cylinderGPU = Geometry::GPU::Cylinder(p.at(i), p.at(i + 4), radius);
        cylinders.push_back(cylinderGPU);

        auto t2 = Geometry::Sphere(this->spheres.size());
        primitives.push_back(t2);
        auto sphereGPU = Geometry::GPU::Sphere(lastBotPoint, radius);
        spheres.push_back(sphereGPU);

        t2 = Geometry::Sphere(this->spheres.size());
        primitives.push_back(t2);
        sphereGPU = Geometry::GPU::Sphere(lastTopPoint, radius);
        spheres.push_back(sphereGPU);

        lastBotPoint = p.at(i);
        lastTopPoint = p.at(i + 4);
    }
    Shape shape;
    shape.name = "SelectedObject";
    shape.shapeParts.push_back(part);
    return shape;
}
