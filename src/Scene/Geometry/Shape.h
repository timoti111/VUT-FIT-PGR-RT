//#pragma once
//#include <string>
//#include <vector>
//#include <memory>
//#include <glm/vec2.hpp>
//#include <glm/vec4.hpp>
//#include <glm/mat4x4.hpp>
//#include "Geometry.h"
//
//namespace Geometry
//{
//    struct Attributes
//    {
//        std::vector<glm::vec4> vertices;
//        std::vector<glm::vec4> normals;
//        std::vector<glm::vec2> coords;
//    };
//
//    struct Shape;
//    struct MeshTest
//    {
//        MeshTest(std::shared_ptr<Shape>& parent);
//        std::string name;
//        std::vector<Geometry::GPU::Triangle> triangles;
//        std::vector<Geometry::GPU::Sphere> spheres;
//        std::vector<Geometry::GPU::Cylinder> cylinders;
//
//        std::vector<Geometry::Mesh> instances;
//        void instantiate(glm::mat4x4 objectToWorld);
//        std::shared_ptr<Shape> parent;
//    };
//
//    struct Shape
//    {
//        std::string name;
//        Attributes attributes;
//        std::vector<MeshTest> meshes;
//
//        std::vector<Geometry::Primitive> primitives;
//        void instantiate(bool asOneObject);
//
//
//        static std::shared_ptr<Shape> fromObjFile(std::string path, std::string name);
//    };
//}