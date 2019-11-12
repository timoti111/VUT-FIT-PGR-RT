#pragma once
#include <geGL/geGL.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <string>
#include <limits>
#include "bvh/BVH.h"

const float kInfinity = std::numeric_limits<float>::max();

class Scene : public BVH
{
public:
    Scene();

    enum PrimitiveTypes
    {
        MESH,
        TRIANGLE,
        SPHERE,
        BOX
    };

    struct alignas(16) Triangle
    {
        glm::ivec4 vertices;
        glm::ivec4 normals;
        glm::ivec4 uvs;
    };

    struct GeomPrimitive : public TreeObject
    {
        GeomPrimitive(GLint type, GLint index);
        AABB getAABB(Scene& scene, glm::mat4x4& objectToWorld);
        glm::vec3 getCentroid(Scene& scene, glm::mat4x4& objectToWorld);

        GLint type;
        GLint index;
    };

    struct alignas(4) GeomPrimitiveGPU
    {
        GLint type;
        GLint index;

        GeomPrimitiveGPU(GeomPrimitive p);
    };

    struct TrianglePrimitive : public GeomPrimitive
    {
        TrianglePrimitive(int index);
    };

    struct Mesh : public GeomPrimitive, public BVH
    {
        Mesh(
            Scene& scene,
            std::string name,
            int primitiveOffset,
            int primitiveCount,
            glm::mat4x4 objectToWorld = glm::mat4x4(1.0f)
        );
        AABB getAABB(Scene& scene, glm::mat4x4& objectToWorld);
        glm::vec3 getCentroid(Scene& scene, glm::mat4x4& objectToWorld);
        void setObjectToWorld(glm::mat4x4& objectToWorld);
        glm::mat4x4 getObjectToWorld();
        void updateBVHs();

        Scene* scene;
        std::string name;
        glm::mat4x4 objectToWorld;
        int gpuPrimitiveOffset;
        int bvhOffset;
    };

    struct alignas(16) MeshGPU
    {
        glm::mat4x4 objectToWorld;
        int primitiveOffset;
        int bvhOffset;
        int materialID;
        bool smoothing;

        MeshGPU(Mesh& o);
    };

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
    };

    void shapeFromObj(std::string path, std::string name);
    void instantiateShape(std::string shapeName, glm::mat4x4 objectToWorld);
    void removeMesh(std::string name);
    void setUpdateSceneBVH();
    void updateBVHs();
    void sceneUpdatedCallback(); // TODO CALLBACK

    std::vector<Scene::Mesh> meshes;
    std::vector<GeomPrimitive> primitives;

    // All these vectors will be sent to GPU
    std::vector<MeshGPU> meshesGPU;
    std::vector<GeomPrimitiveGPU> primitivesGPU;

    std::vector<Triangle> triangles;
    std::vector<glm::vec4> vertices;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec2> coords;

    std::vector<BVHFlatNode> meshBVHs;
    std::vector<BVHFlatNode> sceneBVH;
private:
    std::vector<Shape> shapes;
    bool updateTree = false;
    bool updateSceneBVH = false;
};
