#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Camera {
public:
    Camera();

    void moveLeft(bool moving);
    void moveRight(bool moving);
    void moveFront(bool moving);
    void moveBack(bool moving);
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
    void update(double timeStep);
    void setDirection(glm::vec3 direction);

private:
    glm::vec3 acceleration;
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 left;
    float fov;
    float sensorHalfWidth;
    float aperture;
    float focusDistance;
    float focalLength = 0.00035f;
    float speed = 1.0f;
    bool movingLeft = false;
    bool movingRight = false;
    bool movingFront = false;
    bool movingBack = false;
};