#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <functional>
#include "Geometry.h"
#include "Material.h"

namespace Geometry
{
    struct Attributes
    {
        std::vector<glm::vec4> vertices;
        std::vector<glm::vec4> normals;
        std::vector<glm::vec2> coords;
    };

    struct Shape;
    struct Mesh
    {
        Mesh(std::shared_ptr<Shape>& parent);
        std::string name;
        std::vector<Geometry::Triangle> triangles;
        std::vector<Geometry::Sphere> spheres;
        std::vector<Geometry::Cylinder> cylinders;

        std::vector<std::shared_ptr<Geometry::MeshInstance>> instances;
        void instanceChanged();
        std::shared_ptr<Geometry::MeshInstance> instantiate(glm::mat4x4 objectToWorld, bool smoothing = false, int materialID = -1);
        std::shared_ptr<Shape> parent;
        int materialID = -1;
    };

    struct Shape
    {
        std::string name;
        Attributes attributes;
        std::vector<Mesh> meshes;
        std::vector<Material> materials;
        std::vector<int> materialIDs;
        void meshChanged();
        void instantiate(glm::mat4x4 objectToWorld, bool smoothing = false, int materialID = -1);

        static std::shared_ptr<Shape> fromObjFile(std::string path, std::string name);
    };
}