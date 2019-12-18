#pragma once
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <functional>
#include <map>
#include <string>


struct alignas(16) Light
{
    glm::vec4 sphere;
    int materialID;
};

class Ray;
class Scene;
class Lights
{
public:
    void addLight();
    void setLightsUpdatedCallback(std::function<void()> callback);
    bool tryToSelect(Ray& ray);
    void drawSelectedLight();
    std::vector<Light> lights;

private:
    std::function<void()> lightsUpdatedCallback = [](){};
    int selectedLightIndex = -1;
    std::vector<Light> hiddenLights;
    bool intersectLight(Ray& ray, Light& light);
};

