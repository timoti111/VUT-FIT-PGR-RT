#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Camera {
public:
    Camera();

    void moveCamera(glm::vec3& direction, float delta);
    void rotateCamera(float yaw, float pitch);
    void setFov(float fov);
    void setAperture(float aperture);
    void setFocusDistance(float focusDistance);
    glm::vec3& getPosition();
    glm::vec3& getDirection();
    glm::vec3& getUp();
    glm::vec3& getLeft();
    float getSensorHalfWidth();
    void drawImGui();
    void update();
    void setDirection(glm::vec3 direction);

private:
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 left;
    float fov;
    float sensorHalfWidth;
    float aperture;
    float focusDistance;
    float focalLength = 0.00035f;
};