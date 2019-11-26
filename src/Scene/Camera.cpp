#include "Camera.h"
#include "glm/trigonometric.hpp"
#include "../imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

Camera::Camera()
{
    this->position = glm::vec3(0.0f, 2.0f, 4.0f);
    this->direction = glm::vec3(0.0f, 0.0f, -1.0f);
    this->up = glm::vec3(0.0f, 1.0f, 0.0f);
    this->left = glm::vec3(-1.0f, 0.0f, 0.0f);
    this->acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
    this->aperture;
    this->focusDistance;
    setFov(90.0f);
}

void Camera::moveLeft(bool moving)
{
    movingLeft = moving;
}

void Camera::moveRight(bool moving)
{
    movingRight = moving;
}

void Camera::moveFront(bool moving)
{
    movingFront = moving;
}

void Camera::moveBack(bool moving)
{
    movingBack = moving;
}

void Camera::rotateCamera(float yaw, float pitch)
{
}

void Camera::setFov(float fov)
{
    this->fov = fov;
}

void Camera::setAperture(float aperture)
{
}

void Camera::setFocusDistance(float focusDistance)
{
}

glm::vec3& Camera::getPosition()
{
    return position;
}

glm::vec3& Camera::getDirection()
{
    return direction;
}

glm::vec3& Camera::getUp()
{
    return up;
}

glm::vec3& Camera::getLeft()
{
    return left;
}

float Camera::getSensorHalfWidth()
{
    return sensorHalfWidth;
}

void Camera::drawImGui()
{
    ImGui::Begin("Camera Settings");
    ImGui::Text("Field of view");
    ImGui::SliderFloat("degrees", &fov, 10.0f, 180.0f);
    ImGui::Text("Movement Speed");
    ImGui::InputFloat("m/s", &speed, 0.01f, 1.0f, "%.3f");
    ImGui::End();
}

void Camera::update(double timeStep)
{
    sensorHalfWidth = glm::tan(glm::radians(fov * 0.5f));
    this->position += (this->direction * (float)movingFront - this->direction * (float)movingBack + this->left * (float)movingLeft - this->left * (float)movingRight) * (float)timeStep * speed;
}

void Camera::setDirection(glm::vec3 direction)
{
    glm::mat4x4 mat = glm::lookAt(this->position, this->position + direction, glm::vec3(0.0, 1.0, 0.0));
    this->left = glm::vec3(mat[0][0], mat[1][0], mat[2][0]);
    this->up = glm::vec3(mat[0][1], mat[1][1], mat[2][1]);
    this->direction = glm::vec3(mat[0][2], mat[1][2], mat[2][2]);
}