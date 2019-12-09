#pragma once
#include <glm/vec4.hpp>
#include <functional>

struct alignas(16) Camera
{
    glm::vec4 position;
    glm::vec4 direction;
    glm::vec4 up;
    glm::vec4 left;
    float sensorHalfWidth;
    float focusDistance;
    float aperture;
};

struct alignas(16) RenderParameters
{
    Camera camera;
    int maxBounces;
    bool useEnvironmentMap;
};

class RenderInfo
{
public:
    RenderInfo();

    void moveLeft(float timeStep);
    void moveRight(float timeStep);
    void moveFront(float timeStep);
    void moveBack(float timeStep);
    void setFov(float fov);
    void setAperture(float aperture);
    void setFocusDistance(float focusDistance);
    void drawImGui();
    int getPreviewBounces();
    void setDirection(glm::vec4 direction);
    void rotateCamera(glm::vec2 delta);
    void setParamsUpdatedCallback(std::function<void()> callback);
    RenderParameters renderParams;

private:
    std::function<void()> paramsUpdatedCallback = [](){};
    glm::vec4 acceleration;
    glm::vec2 rotation;
    float fov;
    float speed = 1.0f;
    int previewBounces;
    bool movingLeft = false;
    bool movingRight = false;
    bool movingFront = false;
    bool movingBack = false;
};