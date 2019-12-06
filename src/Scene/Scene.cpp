#include "Scene.h"
#include <random>
#include <glm/gtx/norm.hpp>

Scene::Scene() : BVH(4)
{}

void Scene::addShape(std::shared_ptr<Geometry::Shape> shape)
{
    shapes.push_back(shape);
}

void Scene::instantiateShape(std::string name, glm::mat4x4 objToWorld, int materialID, bool smoothing)
{
    bool found = false;
    for (int i = 0; i < shapes.size(); i++)
    {
        if (shapes[i]->name == name)
        {
            shapes[i]->instantiate(objToWorld, materialID, smoothing);
            found = true;
            break;
        }
    }
    if (!found)
        throw std::runtime_error("Shape of name '" + name + "' not found");
    updateSceneBVH = true;
}

void Scene::removeShape(std::string meshName)
{
    bool found = false;
    for (int i = 0; i < shapes.size(); i++)
    {
        if (shapes[i]->name == meshName)
        {
            found = true;
            shapes.erase(shapes.begin() + i);
            break;
        }
    }
    if (!found)
        throw std::runtime_error("Shape of name '" + meshName + "' not found");
    updateSceneBVH = true;
}

void Scene::selectMesh(Ray& ray)
{
    Geometry::MeshInstance* oldSelectedMesh = actualSelectedMesh;
    actualSelectedMesh = ray.traceRay(this, false);
    if (actualSelectedMesh != oldSelectedMesh)
    {
        updateSelectedMesh = true;
    }
    updateBVHs();
}

//void Scene::selectMesh(int index)
//{
//    if (actualSelectedMesh != -1 || index == -1)
//    {
//        try
//        {
//            removeShape("SelectedObject");
//        }
//        catch (const std::runtime_error& e)
//        {
//        }
//    }
//
//    if (index != -1)
//    {
//        this->shapes.push_back(createSelectedObjectShape(index));
//        instantiateShape("SelectedObject", glm::mat4x4(1.0f), true);
//        //Geometry::Mesh mesh = Geometry::Mesh(
//        //    *this,
//        //    "SelectedObject",
//        //    actualPrimitiveCount,
//        //    20,
//        //    glm::mat4x4(1.0f));
//        //meshes.push_back(mesh);
//
//    }
//    // createMesh "SelectedObject"
//
//    actualSelectedMesh = index;
//    updateBVHs();
//}

void Scene::setUpdateSceneBVH()
{
    updateSceneBVH = true;
}

void Scene::updateBVHs()
{
    if (updateSelectedMesh)
    {
        try
        {
            removeShape("SelectedObject");
        }
        catch (const std::runtime_error & e)
        {
        }

        if (actualSelectedMesh != nullptr)
        {
            createSelectedObjectShape();
            instantiateShape("SelectedObject", glm::mat4x4(1.0f));
        }
        updateSelectedMesh = false;
    }

    if (!updateSceneBVH)
        return;
    updateSceneBVH = false;

    clearPrimitives();
    meshBVHs.clear();
    primitivesGPU.clear();
    triangles.clear();
    spheres.clear();
    cylinders.clear();
    vertices.clear();
    normals.clear();
    coords.clear();
    for (int i = 0; i < shapes.size(); i++)
    {
        auto& shape = shapes[i];
        for (int j = 0; j < shape->meshes.size(); j++)
        {
            auto& mesh = shape->meshes[j];
            for (int k = 0; k < mesh.instances.size(); k++)
            {
                Geometry::MeshInstance* meshInstance = mesh.instances[k].get();
                addPrimitive(meshInstance);
                meshInstance->bvhOffset = meshBVHs.size();
                meshInstance->gpuPrimitiveOffset = primitivesGPU.size();
                auto& meshBVH = meshInstance->getFlatTree();
                meshBVHs.insert(meshBVHs.end(), meshBVH.begin(), meshBVH.end());
                for (auto& treePrims : meshInstance->getPrimitives())
                {
                    auto primitive = Geometry::GPU::Primitive(*dynamic_cast<Geometry::Primitive*>(treePrims));
                    switch (primitive.type)
                    {
                        case Geometry::TRIANGLE:
                            primitive.index += triangles.size();
                            break;
                        case Geometry::SPHERE:
                            primitive.index += spheres.size();
                            break;
                        case Geometry::CYLINDER:
                            primitive.index += cylinders.size();
                            break;
                        default:
                            break;
                    }
                    primitivesGPU.push_back(primitive);
                }
            }

            glm::ivec3 offsets = glm::ivec3(vertices.size(), normals.size(), coords.size());
            for (auto& triangle : mesh.triangles)
                triangles.push_back(Geometry::GPU::Triangle(triangle, offsets));
            for (auto& sphere : mesh.spheres)
            spheres.insert(spheres.end(), mesh.spheres.begin(), mesh.spheres.end());
            cylinders.insert(cylinders.end(), mesh.cylinders.begin(), mesh.cylinders.end());
        }
        vertices.insert(vertices.end(), shape->attributes.vertices.begin(), shape->attributes.vertices.end());
        normals.insert(normals.end(), shape->attributes.normals.begin(), shape->attributes.normals.end());
        coords.insert(coords.end(), shape->attributes.coords.begin(), shape->attributes.coords.end());
    }

    update();

    meshesGPU.clear();
    for (auto& treePrimitive : getPrimitives())
        meshesGPU.push_back(*dynamic_cast<Geometry::MeshInstance*>(treePrimitive));

    sceneUpdatedCallback();

    /*   Geometry::MeshInstance* selectedMesh = nullptr;
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
           meshesGPU.push_back(*dynamic_cast<Geometry::MeshInstance*>(treePrimitive));

       sceneUpdatedCallback();*/


}

void Scene::setSceneUpdateCallback(std::function<void()> callback)
{
    sceneUpdatedCallback = callback;
}

void Scene::createSelectedObjectShape()
{
    if (actualSelectedMesh == nullptr)
        return;

    float radius = 0.01f;
    AABB meshAABB = actualSelectedMesh->getAABB();
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

    auto shape = std::make_shared<Geometry::Shape>();
    shape->name = "SelectedObject";// , static_cast<int>(primitives.size()), 20 };
    Geometry::Mesh mesh(shape);
    mesh.name = "Unnamed";

    for (int i = 0; i < 4; i++)
    {
        mesh.cylinders.push_back(Geometry::Cylinder(p.at(i), lastBotPoint, radius));
        mesh.cylinders.push_back(Geometry::Cylinder(p.at(i + 4), lastTopPoint, radius));
        mesh.cylinders.push_back(Geometry::Cylinder(p.at(i), p.at(i + 4), radius));
        mesh.spheres.push_back(Geometry::Sphere(lastBotPoint, radius));
        mesh.spheres.push_back(Geometry::Sphere(lastTopPoint, radius));

        lastBotPoint = p.at(i);
        lastTopPoint = p.at(i + 4);
    }
    shape->meshes.push_back(mesh);
    shapes.push_back(shape);
}
