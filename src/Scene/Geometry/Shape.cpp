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
            std::cout << image.size() << " " << material.normal_texname << " " << width << " x " << height << std::endl;

            //if there's an error, display it
            if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
            auto texture = std::make_shared<ge::gl::Texture>(GL_TEXTURE_2D, GL_RGBA32F, 0, width, height);
            texture->setData2D(image.data(), GL_RGBA, GL_UNSIGNED_BYTE);
            texture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            texture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            newMaterial->map_N = Textures::addTexture(texture);
        }
        newMaterial->type = IDEAL_REFLECTION;
        //newMaterial.map_Kd = generateTexture(dir + material.diffuse_texname);
        //newMaterial.map_N = generateTexture(dir + material.diffuse_texname);
        //shapeInternal->materials.push_back(newMaterial);
        //shapeInternal->materialIDs.push_back(Material::getNextMaterialID());
    }
    shapeInternal->name = name;
    for (auto& shape : shapes)
    {
        //if (shape.name.empty())
        //    throw std::runtime_error("Unnamed object in file: " + path);

        Mesh mesh(shapeInternal);
        mesh.name = shape.name.empty() ? "Unnamed" : shape.name;
        mesh.materialID = shape.mesh.material_ids.front() + materialIDOffset;
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3)
        {
            mesh.triangles.push_back(Geometry::Triangle(
                glm::ivec4(glm::ivec3(shape.mesh.indices[i].vertex_index, shape.mesh.indices[i + 1].vertex_index, shape.mesh.indices[i + 2].vertex_index), 0),
                glm::ivec4(glm::ivec3(shape.mesh.indices[i].normal_index, shape.mesh.indices[i + 1].normal_index, shape.mesh.indices[i + 2].normal_index), 0),
                glm::ivec4(glm::ivec3(shape.mesh.indices[i].texcoord_index, shape.mesh.indices[i + 1].texcoord_index, shape.mesh.indices[i + 2].texcoord_index), 0)
            )
            );
        }
        shapeInternal->meshes.push_back(mesh);
    }

    for (size_t i = 0; i < attrib.vertices.size(); i += 3)
    {
        shapeInternal->attributes.vertices.push_back(glm::vec4(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2], 1.0f));
    }

    for (size_t i = 0; i < attrib.normals.size(); i += 3)
    {
        shapeInternal->attributes.normals.push_back(glm::vec4(attrib.normals[i], attrib.normals[i + 1], attrib.normals[i + 2], 0.0f));
    }

    for (size_t i = 0; i < attrib.texcoords.size(); i += 2)
    {
        shapeInternal->attributes.coords.push_back(glm::vec2(attrib.texcoords[i], attrib.texcoords[i + 1]));
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
