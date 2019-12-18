#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <imgui.h>
#include "RenderInfo.h"
#include "glm/trigonometric.hpp"
#include <iostream>

RenderInfo::RenderInfo()
{
    renderParams.camera.position = glm::vec4(0.0f, 2.0f, 4.0f, 1.0f);
    renderParams.camera.direction = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    renderParams.camera.up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    renderParams.camera.left = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
    rotation = glm::vec2(0.0f, 0.0f);
    renderParams.camera.aperture = 0.0f;
    renderParams.camera.focusDistance = 1.0f;
    renderParams.maxBounces = 8;
    renderParams.backgroundIntensity = 1.0f;
    renderParams.backgroundColor = glm::vec4(1.0f);
    renderParams.useEnvironmentMap = true;
    renderParams.useRussianRoulette = true;
    renderParams.sampleDirect = true;
    renderParams.numberOfLights = 0;
    previewBounces = 2;
    setFov(90.0f);
}

void RenderInfo::moveLeft(float timeStep)
{
    renderParams.camera.position += renderParams.camera.left * timeStep * speed;
    paramsUpdatedCallback();
}

void RenderInfo::moveRight(float timeStep)
{
    renderParams.camera.position -= renderParams.camera.left * timeStep * speed;
    paramsUpdatedCallback();
}

void RenderInfo::moveFront(float timeStep)
{
    renderParams.camera.position += renderParams.camera.direction * timeStep * speed;
    paramsUpdatedCallback();
}

void RenderInfo::moveBack(float timeStep)
{
    renderParams.camera.position -= renderParams.camera.direction * timeStep * speed;
    paramsUpdatedCallback();
}

void RenderInfo::setFov(float fov)
{
    if (this->fov != fov)
    {
        this->fov = fov;
        renderParams.camera.sensorHalfWidth = glm::tan(glm::radians(fov * 0.5f));
        paramsUpdatedCallback();
    }
}

void RenderInfo::setBackgroundColor(glm::vec4 color)
{
    if (renderParams.backgroundColor != color)
    {
        renderParams.backgroundColor = color;
        paramsUpdatedCallback();
    }
}

void RenderInfo::setBackgroundIntensity(float intensity)
{
    if (renderParams.backgroundIntensity != intensity)
    {
        renderParams.backgroundIntensity = intensity;
        paramsUpdatedCallback();
    }
}

void RenderInfo::setMaxBounces(int bounces)
{
    if (renderParams.maxBounces != bounces && bounces > 0)
    {
        renderParams.maxBounces = bounces;
        paramsUpdatedCallback();
    }
}

void RenderInfo::setUseEnvironmentMap(bool useEnvironmentMap)
{
    if (renderParams.useEnvironmentMap != useEnvironmentMap)
    {
        renderParams.useEnvironmentMap = useEnvironmentMap;
        paramsUpdatedCallback();
    }
}

void RenderInfo::setUseRussianRoulette(bool useRussianRoulette)
{
    if (renderParams.useRussianRoulette != useRussianRoulette)
    {
        renderParams.useRussianRoulette = useRussianRoulette;
        paramsUpdatedCallback();
    }
}

void RenderInfo::setAperture(float aperture)
{
    if (renderParams.camera.aperture != aperture)
    {
        renderParams.camera.aperture = aperture;
        paramsUpdatedCallback();
    }
}

void RenderInfo::setFocusDistance(float focusDistance)
{
    if (renderParams.camera.focusDistance != focusDistance)
    {
        renderParams.camera.focusDistance = focusDistance;
        paramsUpdatedCallback();
    }
}

void RenderInfo::drawGui()
{
    bool useEnvironmentMap = renderParams.useEnvironmentMap;
    bool useRussianRoulette = renderParams.useRussianRoulette;
    bool sampleDirect = renderParams.sampleDirect;
    bool changed = false;

    ImGui::Begin("Render Settings");
    ImGui::Text("Max bounces");
    changed |= ImGui::InputInt("bounces", &renderParams.maxBounces);
    changed |= ImGui::Checkbox("Use russian roulette", &useRussianRoulette);
    changed |= ImGui::Checkbox("Sample direct light", &sampleDirect);
    if (ImGui::CollapsingHeader("Background settings"))
    {
        changed |= ImGui::ColorEdit3("Background color", &renderParams.backgroundColor.x);
        changed |= ImGui::Checkbox("Use environment map", &useEnvironmentMap);
        ImGui::Text("Background Intensity");
        changed |= ImGui::SliderFloat("", &renderParams.backgroundIntensity, 0.0f, 100.0f);
    }
    if (ImGui::CollapsingHeader("Camera settings"))
    {
        ImGui::Text("Field of view");
        bool fovChanged = ImGui::SliderFloat("degrees", &fov, 1.0f, 179.9f);
        if (fovChanged)
            renderParams.camera.sensorHalfWidth = glm::tan(glm::radians(fov * 0.5f));
        changed |= fovChanged;
        ImGui::Text("Aperture Diameter");
        changed |= ImGui::SliderFloat("cm", &renderParams.camera.aperture, 0.0f, 20.0f);
        ImGui::Text("Movement Speed");
        changed |= ImGui::InputFloat("m/s", &speed, 0.01f, 1.0f, "%.3f");
    }
    ImGui::End();

    renderParams.useEnvironmentMap = useEnvironmentMap;
    renderParams.useRussianRoulette = useRussianRoulette;
    renderParams.sampleDirect = sampleDirect;

    if (changed)
        paramsUpdatedCallback();
}

int RenderInfo::getPreviewBounces()
{
    return previewBounces;
}

void RenderInfo::rotateCamera(glm::vec2 delta)
{
    static auto zeroDelta = glm::vec2(0.0f);
    static auto ident = glm::mat4x4(1.0f);

    rotation += delta * 0.1f;
    if (rotation.x < 0.0f) rotation.x += 360.0f;
    if (rotation.y < 0.0f) rotation.y += 360.0f;
    if (rotation.x > 360.0f) rotation.x -= 360.0f;
    if (rotation.y > 360.0f) rotation.y -= 360.0f;
    auto mat = glm::rotate(ident, glm::radians(rotation.y), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(ident, glm::radians(rotation.x), glm::vec3(0.0f, 1.0f, 0.0f));
    renderParams.camera.left = glm::normalize(glm::vec4(mat[0][0], mat[1][0], mat[2][0], 0.0f));
    renderParams.camera.up = glm::normalize(glm::vec4(mat[0][1], mat[1][1], mat[2][1], 0.0f));
    renderParams.camera.direction = glm::normalize(glm::vec4(mat[0][2], mat[1][2], mat[2][2], 0.0f));
    paramsUpdatedCallback();
}

void RenderInfo::setParamsUpdatedCallback(std::function<void()> callback)
{
    paramsUpdatedCallback = callback;
}
