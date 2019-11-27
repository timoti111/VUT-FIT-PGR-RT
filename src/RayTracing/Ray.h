#pragma once
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

class Scene;
class Camera;
namespace Geometry
{
    class MeshInstance;
}
struct Ray
{
    glm::vec4 origin;
    glm::vec4 direction;
    float t;
    Geometry::MeshInstance* traceRay(Scene* scene, bool occlusion);
    static Ray createCameraRay(Camera* camera, glm::vec2 pixel, glm::ivec2 resolution);
};