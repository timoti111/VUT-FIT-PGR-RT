#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include "Geometry.h"

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
        void instantiate(glm::mat4x4 objectToWorld);
        std::shared_ptr<Shape> parent;
    };

    struct Shape
    {
        std::string name;
        Attributes attributes;
        std::vector<Mesh> meshes;

        void meshChanged();
        void instantiate(glm::mat4x4 objectToWorld);

        static std::shared_ptr<Shape> fromObjFile(std::string path, std::string name);
    };
}