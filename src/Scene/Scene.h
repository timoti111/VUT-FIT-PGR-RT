#pragma once
#include <geGL/geGL.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <string>
#include <limits>
#include <functional>
#include "Geometry/Shape.h"

const float kInfinity = std::numeric_limits<float>::max();

class Scene : public BVH
{
public:
    Scene();

    void addShape(std::shared_ptr<Geometry::Shape> shape);
    void instantiateShape(std::string name, glm::mat4x4 objToWorld = glm::mat4x4(1.0f), int materialID = -1, bool smoothing = true);
    void removeShape(std::string name);
    void selectMesh(Ray& ray);
    void setUpdateSceneBVH();
    void updateBVHs();
    void setSceneUpdateCallback(std::function<void()> callback);


    // All these vectors will be sent to GPU
    std::vector<Geometry::GPU::MeshInstance> meshesGPU;
    std::vector<Geometry::GPU::Primitive> primitivesGPU;
    std::vector<glm::vec4> vertices;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec2> coords;

    std::vector<Geometry::GPU::Triangle> triangles;

    std::vector<Geometry::Sphere> spheres;

    std::vector<Geometry::Cylinder> cylinders;

    std::vector<BVHFlatNode> meshBVHs;
private:
    std::vector<std::shared_ptr<Geometry::Shape>> shapes;
    void createSelectedObjectShape();

protected:
    std::function<void()> sceneUpdatedCallback;
    bool updateSceneBVH = false;
    bool updateSelectedMesh = false;
    Geometry::MeshInstance* actualSelectedMesh = nullptr;
};
