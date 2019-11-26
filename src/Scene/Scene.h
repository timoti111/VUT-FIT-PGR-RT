#pragma once
#include <geGL/geGL.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <string>
#include <limits>
#include "Geometry/Geometry.h"

const float kInfinity = std::numeric_limits<float>::max();

class Scene : public BVH
{
public:
    Scene();

    struct ShapePart
    {
        std::string name;
        int primitiveOffset;
        int primitiveCount;
    };

    struct Shape
    {
        // TODO ShapeBuilder
        std::string name;
        std::vector<ShapePart> shapeParts;
        std::vector<int> references;
    };

    void shapeFromObj(std::string path, std::string name);
    void instantiateShape(std::string shapeName, glm::mat4x4 objectToWorld, bool asOneMesh);
    void removeShape(std::string name);
    void selectMesh(int index);
    void setUpdateSceneBVH();
    void updateBVHs();
    void sceneUpdatedCallback(); // TODO CALLBACK

    std::vector<Geometry::Mesh> meshes;
    std::vector<Geometry::Primitive> primitives;

    // All these vectors will be sent to GPU
    std::vector<Geometry::GPU::Mesh> meshesGPU;
    std::vector<Geometry::GPU::Primitive> primitivesGPU;

    std::vector<Geometry::GPU::Triangle> triangles;
    std::vector<glm::vec4> vertices;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec2> coords;

    std::vector<Geometry::GPU::Sphere> spheres;

    std::vector<Geometry::GPU::Cylinder> cylinders;

    std::vector<BVHFlatNode> meshBVHs;
private:
    std::vector<Shape> shapes;
    bool updateSceneBVH = false;
    int actualSelectedMesh = -1;
    Shape createSelectedObjectShape(int index);
};
