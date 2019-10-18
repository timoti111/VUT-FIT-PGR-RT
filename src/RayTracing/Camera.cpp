#include "Camera.h"
#include "glm/trigonometric.hpp"
#include "../imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

Camera::Camera()
{
    this->position = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    this->direction = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    this->up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    this->left = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
    this->aperture;
    this->focusDistance;
    setFov(110.0f);
}

void Camera::moveCamera(glm::vec3& direction, float delta)
{
    this->position += direction * delta;
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
    ImGui::SliderFloat("float", &fov, 10.0f, 130.0f);
    ImGui::End();
}

void Camera::update()
{
    sensorHalfWidth = glm::tan(glm::radians(fov * 0.5f));
}

void Camera::setDirection(glm::vec3 direction)
{
    glm::mat4x4 mat = glm::lookAt(this->position, this->position + direction, glm::vec3(0.0, 1.0, 0.0));
    this->left = glm::vec3(mat[0][0], mat[1][0], mat[2][0]);
    this->up = glm::vec3(mat[0][1], mat[1][1], mat[2][1]);
    this->direction = glm::vec3(mat[0][2], mat[1][2], mat[2][2]);
}
