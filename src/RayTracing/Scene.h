#pragma once
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>

class Scene {
public:
    Scene();
    const glm::vec2 sphereRadius = glm::vec2(3.0f, 8.0f);
    const unsigned spheresMax = 100;
    const float spherePlacementRadius = 100.0f;

    struct Sphere {
        glm::vec3 position;
        float radius;
        glm::vec3 color;
    };

    void generateScene();
private:
    std::vector<Sphere> spheres;
};