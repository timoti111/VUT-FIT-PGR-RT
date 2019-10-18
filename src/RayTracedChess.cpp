#include "RayTracedChess.h"
#include <iostream>
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include "Utils/filetostring.h"
#include <geGL/StaticCalls.h>

RayTracedChess::RayTracedChess() : Application()
{
    vars.add<ge::gl::Program>("computeProgram",
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Utils::readFileToString("res/shaders/computeShader.glsl")));
    vars.add<ge::gl::Program>("basicDrawProgram",
        std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER, Utils::readFileToString("res/shaders/basicVertexShader.glsl")),
        std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER, Utils::readFileToString("res/shaders/basicFragmentShader.glsl")));
;
    float data[] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f };
    vars.add<ge::gl::Buffer>("backgroundQuad", sizeof(float) * 8, data);
    vars.add<ge::gl::VertexArray>("backgroundVAO")->addAttrib(
        vars.get<ge::gl::Buffer>("backgroundQuad"), 0, 2, GL_FLOAT);

    HDRLoader::load("res/textures/delta_2_4k.hdr", backgroundTexture);
    vars.add<ge::gl::Texture>("enviromentTexture", GL_TEXTURE_2D, GL_RGB32F, 0, backgroundTexture.width, backgroundTexture.height)
        ->setData2D(backgroundTexture.cols.data(), GL_RGB, GL_FLOAT);
    vars.get<ge::gl::Texture>("enviromentTexture")->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    vars.get<ge::gl::Texture>("enviromentTexture")->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    vars.get<ge::gl::Texture>("enviromentTexture")->bind(2);

    initComputeShaderImage();

    GLint workGroupSize[3];
    vars.get<ge::gl::Program>("computeProgram")->getContext().glGetProgramiv(vars.get<ge::gl::Program>("computeProgram")->getId(), GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
    workGroupSizeX = workGroupSize[0];
    workGroupSizeY = workGroupSize[1];
    workGroupSizeZ = workGroupSize[2];
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void RayTracedChess::initComputeShaderImage()
{
    vars.reCreate<ge::gl::Texture>("computeShaderImage", GL_TEXTURE_2D, GL_RGBA32F, 0, width, height)
        ->setData2D(backgroundTexture.cols.data(), GL_RGBA, GL_FLOAT);
    vars.get<ge::gl::Texture>("computeShaderImage")->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    vars.get<ge::gl::Texture>("computeShaderImage")->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    vars.get<ge::gl::Texture>("computeShaderImage")->bindImage(1, 0, GL_RGBA32F, GL_WRITE_ONLY);
    vars.get<ge::gl::Texture>("computeShaderImage")->bind(0);
}

void RayTracedChess::draw()
{
    camera.update();
    vars.get<ge::gl::Program>("computeProgram")->set("camera.direction", camera.getDirection().x, camera.getDirection().y, camera.getDirection().z);
    vars.get<ge::gl::Program>("computeProgram")->set("camera.up", camera.getUp().x, camera.getUp().y, camera.getUp().z);
    vars.get<ge::gl::Program>("computeProgram")->set("camera.left", camera.getLeft().x, camera.getLeft().y, camera.getLeft().z);
    vars.get<ge::gl::Program>("computeProgram")->set("camera.position", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
    vars.get<ge::gl::Program>("computeProgram")->set("camera.sensorHalfWidth", camera.getSensorHalfWidth());
    vars.get<ge::gl::Program>("computeProgram")->dispatch(ceil(width / static_cast<float>(workGroupSizeX)), ceil(height / static_cast<float>(workGroupSizeY)), 1);

    vars.get<ge::gl::VertexArray>("backgroundVAO")->bind();
    vars.get<ge::gl::Program>("basicDrawProgram")->use();
    ge::gl::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    vars.get<ge::gl::VertexArray>("backgroundVAO")->unbind();

    drawGui(drawGuiB);

    swap();
}

void RayTracedChess::mouseButtonEvent(int button, int action, int mods)
{
}

void RayTracedChess::mouseMoveEvent(double xpos, double ypos)
{
    static double lastX, lastY;
    static double firstMouse = true;
    static float yaw = 0.0f, pitch = 0.0f;
    if (drawGuiB)
        return;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera.setDirection(glm::normalize(front));
}

void RayTracedChess::keyEvent(int key, int scancode, int action, int mods)
{
    static int moveMultiplier = 20;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        drawGuiB = !drawGuiB;
        glfwSetInputMode(window, GLFW_CURSOR, drawGuiB ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
    if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS))
        camera.moveCamera(camera.getDirection(), deltaTime * moveMultiplier);
    if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
        camera.moveCamera(camera.getLeft(), deltaTime * moveMultiplier);
    if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
        camera.moveCamera(camera.getDirection(), -deltaTime * moveMultiplier);
    if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
        camera.moveCamera(camera.getLeft(), -deltaTime * moveMultiplier);
    if (key == GLFW_KEY_Q && (action == GLFW_REPEAT || action == GLFW_PRESS))
        camera.moveCamera(camera.getUp(), deltaTime * moveMultiplier);
    if (key == GLFW_KEY_E && (action == GLFW_REPEAT || action == GLFW_PRESS))
        camera.moveCamera(camera.getUp(), -deltaTime * moveMultiplier);
}

void RayTracedChess::resizeEvent(int x, int y)
{
    initComputeShaderImage();
}

void RayTracedChess::drawGui(bool drawGui)
{
    if (!drawGui)
        return;

    ImGui::Begin("App info");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    camera.drawImGui();
}
