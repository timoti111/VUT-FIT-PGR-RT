#include "Lights.h"
#include "Material.h"
#include <imgui.h>
#include "Scene/Scene.h"

void Lights::addLight()
{
    if (hiddenLights.empty())
    {
        Light light;
        light.sphere = glm::vec4(0.0f, 1.5f, 0.0f, 0.2f);
        auto material = Material::generateNewMaterial(light.materialID);
        material->type = EMISSIVE;
        lights.push_back(light);
    }
    else
    {
        lights.push_back(hiddenLights.back());
        hiddenLights.erase(hiddenLights.end() - 1);
    }
    lightsUpdatedCallback();
    selectedLightIndex = -1;
}

void Lights::setLightsUpdatedCallback(std::function<void()> callback)
{
    lightsUpdatedCallback = callback;
}

bool Lights::tryToSelect(Ray& ray)
{
    selectedLightIndex = -1;
    for (int i = 0; i < lights.size(); i++)
    {
        if (intersectLight(ray, lights[i]))
            selectedLightIndex = i;
    }
    return selectedLightIndex != -1;
}

void Lights::drawSelectedLight()
{
    if (selectedLightIndex == -1)
        return;

    bool changed = false;
    auto material = Material::getMaterial(lights[selectedLightIndex].materialID);
    ImGui::Begin("Light Settings");
    changed |= ImGui::ColorEdit3("Light Color", &material->Ke.x);
    changed |= ImGui::SliderFloat("Light Strength", &material->Ns, 0.0f, 1000.0f);
    changed |= ImGui::DragFloat3("Light Position", &lights[selectedLightIndex].sphere.x, 0.1f);
    changed |= ImGui::SliderFloat("Light Radius", &lights[selectedLightIndex].sphere.w, 0.0f, 1000.0f);
    if (ImGui::Button("Hide Light"))
    {
        hiddenLights.push_back(lights[selectedLightIndex]);
        lights.erase(lights.begin() + selectedLightIndex);
        selectedLightIndex = -1;
        changed = true;
    }
    ImGui::End();

    if (changed)
        lightsUpdatedCallback();
}

bool Lights::intersectLight(Ray& ray, Light& light)
{
    glm::vec3 sphereDir = light.sphere - ray.origin;
    float tca = glm::dot(sphereDir, glm::vec3(ray.direction));
    float d2 = dot(sphereDir, sphereDir) - tca * tca;
    float radius2 = glm::pow(light.sphere.w, 2);
    if (d2 > radius2)
        return false;
    float thc = sqrt(radius2 - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

    if (t0 < 0.0f)
        t0 = t1; // if t0 is negative, let's use t1 instead 

    if (t0 > 0.0f && t0 < ray.t)
    {
        ray.t = t0;
        return true;
    }
    return false;
}
