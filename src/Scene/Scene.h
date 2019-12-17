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
#include "Geometry/Material.h"
#include <utility>
#include "Geometry/Lights.h"

const float kInfinity = std::numeric_limits<float>::max();

class Scene : public BVH
{
public:
    Scene();

    void addShape(std::shared_ptr<Geometry::Shape> shape);
    void instantiateShape(std::string name, int materialID = -1, bool smoothing = true, glm::mat4x4 objToWorld = glm::mat4x4(1.0f));
    void removeShape(std::string name);
    void selectMesh(Ray& ray);
    void setUpdateSceneBVH();
    void updateBVHs();
    void setSceneUpdateCallback(std::function<void(bool)> callback);
    void drawMaterialSettings();
    void drawLightSettings();
    void addLight();


    std::map<std::string, int> drawableMaterials;
    // All these vectors will be sent to GPU
    std::vector<Geometry::GPU::MeshInstance> meshesGPU;
    std::vector<Geometry::GPU::Primitive> primitivesGPU;

    std::vector<Geometry::Triangle> triangles;

    std::vector<Geometry::Sphere> spheres;

    std::vector<Geometry::Cylinder> cylinders;

    std::vector<BVHFlatNode> meshBVHs;

    Lights lights;
private:
    std::vector<std::shared_ptr<Geometry::Shape>> shapes;
    void createSelectedObjectShape();
    void createLightShape();
    

protected:
    std::function<void(bool)> sceneUpdatedCallback;
    bool updateSceneBVH = false;
    bool updateSelectedMesh = false;
    Geometry::MeshInstance* actualSelectedMesh = nullptr;
    Geometry::MeshInstance* selectedLight = nullptr;
    int selectedObjectMaterial;
};
