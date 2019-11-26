//#include "Shape.h"
//#define TINYOBJLOADER_IMPLEMENTATION
//#include "Utils/tiny_obj_loader.h"
//
//std::shared_ptr<Geometry::Shape> Geometry::Shape::fromObjFile(std::string path, std::string name)
//{
//    tinyobj::attrib_t attrib;
//    std::vector<tinyobj::shape_t> shapes;
//    std::vector<tinyobj::material_t> materials;
//
//
//    std::string warn;
//    std::string err;
//    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
//    {
//        throw std::runtime_error(err);
//    }
//
//    auto shapeInternal = std::make_shared<Geometry::Shape>();
//    shapeInternal->name = name;
//    for (auto& shape : shapes)
//    {
//        //if (shape.name.empty())
//        //    throw std::runtime_error("Unnamed object in file: " + path);
//
//        MeshTest mesh(shapeInternal);
//        mesh.name = shape.name.empty() ? "Unnamed" : shape.name;
//        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
//        {
//            mesh.triangles.push_back(Geometry::GPU::Triangle{
//                glm::ivec4(glm::ivec3(shape.mesh.indices[i].vertex_index, shape.mesh.indices[i + 1].vertex_index, shape.mesh.indices[i + 2].vertex_index), 0),
//                glm::ivec4(glm::ivec3(shape.mesh.indices[i].normal_index, shape.mesh.indices[i + 1].normal_index, shape.mesh.indices[i + 2].normal_index), 0),
//                glm::ivec4(glm::ivec3(shape.mesh.indices[i].texcoord_index, shape.mesh.indices[i + 1].texcoord_index, shape.mesh.indices[i + 2].texcoord_index), 0)
//                                     }
//            );
//        }
//        shapeInternal->meshes.push_back(mesh);
//    }
//
//    for (size_t i = 0; i < attrib.vertices.size(); i += 3)
//    {
//        shapeInternal->attributes.vertices.push_back(glm::vec4(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2], 1.0f));
//    }
//
//    for (size_t i = 0; i < attrib.normals.size(); i += 3)
//    {
//        shapeInternal->attributes.normals.push_back(glm::vec4(attrib.normals[i], attrib.normals[i + 1], attrib.normals[i + 2], 0.0f));
//    }
//
//    for (size_t i = 0; i < attrib.texcoords.size(); i += 2)
//    {
//        shapeInternal->attributes.coords.push_back(glm::vec2(attrib.texcoords[i], attrib.texcoords[i + 1]));
//    }
//
//    return shapeInternal;
//}
//
//void Geometry::MeshTest::instantiate(glm::mat4x4 objectToWorld)
//{
//    //Geometry::Mesh mesh;
//    //mesh.objectToWorld = objectToWorld;
//    //for (auto& triangle : triangles)
//    //{
//    //    mesh.addPrimitive
//    //}
//}
//
//Geometry::MeshTest::MeshTest(std::shared_ptr<Shape>& parent) :
//    parent(parent)
//{}
