#include "Shape.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "Utils/tiny_obj_loader.h"
#include "Scene/Geometry/Textures.h"
#include "lodepng.h"

void Geometry::Shape::meshChanged()
{}

void Geometry::Shape::instantiate(glm::mat4x4 objectToWorld, bool smoothing, int materialID)
{
    for (auto& mesh : meshes)
        mesh.instantiate(objectToWorld, smoothing, materialID);
}

std::shared_ptr<Geometry::Shape> Geometry::Shape::fromObjFile(std::string path, std::string name)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;
    std::string dir = path.substr(0, path.find_last_of('/') + 1);
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), dir.c_str()))
    {
        throw std::runtime_error(err);
    }

    auto shapeInternal = std::make_shared<Geometry::Shape>();
    int materialIDOffset = Material::getMaterials().size();
    for (auto& material : materials)
    {
        int materialID;
        auto newMaterial = Material::generateNewMaterial(materialID);
        if (!material.diffuse_texname.empty())
        {
            std::vector<unsigned char> image; //the raw pixels
            unsigned width, height;
            unsigned error = lodepng::decode(image, width, height, dir + material.diffuse_texname);

            //if there's an error, display it
            if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
            auto texture = std::make_shared<ge::gl::Texture>(GL_TEXTURE_2D, GL_RGBA32F, 0, width, height);
            texture->setData2D(image.data(), GL_RGBA, GL_UNSIGNED_BYTE);
            texture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            texture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            newMaterial->map_Kd = Textures::addTexture(texture);
            newMaterial->map_Ks = newMaterial->map_Kd;
        }
        if (!material.normal_texname.empty())
        {
            std::vector<unsigned char> image; //the raw pixels
            unsigned width, height;
            unsigned error = lodepng::decode(image, width, height, dir + material.normal_texname);

            //if there's an error, display it
            if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
            auto texture = std::make_shared<ge::gl::Texture>(GL_TEXTURE_2D, GL_RGBA32F, 0, width, height);
            texture->setData2D(image.data(), GL_RGBA, GL_UNSIGNED_BYTE);
            texture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            texture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            newMaterial->map_N = Textures::addTexture(texture);
        }
    }
    shapeInternal->name = name;
    for (auto& shape : shapes)
    {
        Mesh mesh(shapeInternal);
        mesh.name = shape.name.empty() ? "Unnamed" : shape.name;
        mesh.materialID = shape.mesh.material_ids.front() + materialIDOffset;
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
        {
            Geometry::Triangle triangle;
            auto vIndices = glm::ivec3(shape.mesh.indices[i].vertex_index, shape.mesh.indices[i + 1].vertex_index, shape.mesh.indices[i + 2].vertex_index) * 3;
            auto nIndices = glm::ivec3(shape.mesh.indices[i].normal_index, shape.mesh.indices[i + 1].normal_index, shape.mesh.indices[i + 2].normal_index) * 3;
            auto cIndices = glm::ivec3(shape.mesh.indices[i].texcoord_index, shape.mesh.indices[i + 1].texcoord_index, shape.mesh.indices[i + 2].texcoord_index) * 2;
            for (int j = 0; j < 3; j++)
            {
                triangle.vertices[j] = glm::vec4(attrib.vertices[vIndices[j]], attrib.vertices[vIndices[j] + 1], attrib.vertices[vIndices[j] + 2], 1.0f);
                if (nIndices.x >= 0)
                    triangle.normals[j] = glm::vec4(attrib.normals[nIndices[j]], attrib.normals[nIndices[j] + 1], attrib.normals[nIndices[j] + 2], 0.0f);
                if (cIndices.x >= 0)
                    triangle.coords[j] = glm::vec2(attrib.texcoords[cIndices[j]], attrib.texcoords[cIndices[j] + 1]);
            }
            mesh.triangles.push_back(triangle);
        }
        shapeInternal->meshes.push_back(mesh);
    }

    return shapeInternal;
}

void Geometry::Mesh::instanceChanged()
{
    parent->meshChanged();
}

std::shared_ptr<Geometry::MeshInstance> Geometry::Mesh::instantiate(glm::mat4x4 objectToWorld, bool smoothing, int materialID)
{
    if (materialID != -1)
        this->materialID = materialID;
    auto mesh = std::make_shared<Geometry::MeshInstance>(this, objectToWorld, this->materialID, smoothing);
    instances.push_back(mesh);
    return mesh;
}

Geometry::Mesh::Mesh(std::shared_ptr<Shape>& parent) :
    parent(parent)
{}
