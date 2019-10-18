#include "Scene.h"
#include <random>
#include <glm/gtx/norm.hpp>

Scene::Scene()
{
}

void Scene::generateScene()
{
    std::default_random_engine generator;
    std::uniform_real_distribution<double> rand(0.0, 1.0);

    // Add a number of random spheres
    for (int i = 0; i < spheresMax; i++)
    {
        Sphere sphere;
        // Radius and radius
        sphere.radius = sphereRadius.x + rand(generator) * (sphereRadius.y - sphereRadius.x);
        glm::vec2 randomPos = glm::vec2(rand(generator) * spherePlacementRadius, rand(generator) * spherePlacementRadius);
        sphere.position = glm::vec3(randomPos.x, sphere.radius, randomPos.y);

        // Reject spheres that are intersecting others
        for (auto& other : spheres)
        {
            float minDist = sphere.radius + other.radius;
            if (glm::length2(sphere.position - other.position) < minDist * minDist)
                goto SkipSphere;
        }

        // Albedo and specular color
        sphere.color = glm::vec3(rand(generator), rand(generator), rand(generator));
        //bool metal = Random.value < 0.5f;
        //sphere.albedo = metal ? Vector4.zero : new Vector4(color.r, color.g, color.b);
        //sphere.specular = metal ? new Vector4(color.r, color.g, color.b) : new Vector4(0.04f, 0.04f, 0.04f);

        // Add the sphere to the list
        spheres.push_back(sphere);

    SkipSphere:
        continue;
    }
}
