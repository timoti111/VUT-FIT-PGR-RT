﻿#include "RayTracedChess.h"
#include <iostream>
#include <cmath>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Utils/filetostring.h"
#include <geGL/StaticCalls.h>
#include "imgui.h"
#include "bvh/BVH.h"
#include <glm/gtc/matrix_transform.hpp>

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
    vars.add<ge::gl::Texture>("enviromentTexture", GL_TEXTURE_2D, GL_RGB32F, 10, backgroundTexture.width, backgroundTexture.height)
        ->setData2D(backgroundTexture.cols.data(), GL_RGB, GL_FLOAT);
    vars.get<ge::gl::Texture>("enviromentTexture")->generateMipmap();
    vars.get<ge::gl::Texture>("enviromentTexture")->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    vars.get<ge::gl::Texture>("enviromentTexture")->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    vars.get<ge::gl::Texture>("enviromentTexture")->bind(2);

    initComputeShaderImage();

    GLint workGroupSize[3];
    ge::gl::glGetProgramiv(vars.get<ge::gl::Program>("computeProgram")->getId(), GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
    vars.addInt32("workGroupSizeX", workGroupSize[0]);
    vars.addInt32("workGroupSizeY", workGroupSize[1]);
    vars.addInt32("workGroupSizeZ", workGroupSize[2]);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    auto ident = glm::mat4x4(1.0f);
    double actTime = glfwGetTime();
    scene.shapeFromObj("res/models/bunny/bunny.obj", "Bunny");
    double time = glfwGetTime() - actTime;
    std::cout << "Model load time: " << time << std::endl;
    actTime = glfwGetTime();
    scene.instantiateShape("Bunny", ident, true);
    time = glfwGetTime() - actTime;
    std::cout << "BVH build time: " << time << std::endl;
    scene.updateBVHs();

    vars.add<ge::gl::Buffer>("sceneBVHBuffer", scene.getFlatTree(), GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 3);
    vars.add<ge::gl::Buffer>("meshBHVs", scene.meshBVHs, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 4);
    vars.add<ge::gl::Buffer>("meshesBuffer", scene.meshesGPU, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 5);
    vars.add<ge::gl::Buffer>("primitivesBuffer", scene.primitivesGPU, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 6);
    vars.add<ge::gl::Buffer>("trianglesBuffer", scene.triangles, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 7);
    vars.add<ge::gl::Buffer>("verticesBuffer", scene.vertices, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 8);
    vars.add<ge::gl::Buffer>("normalsBuffer", scene.normals, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 9);
    vars.add<ge::gl::Buffer>("coordsBuffer", scene.coords, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 10);
}

void RayTracedChess::initComputeShaderImage()
{
    static std::vector<GLfloat> emptyData(width * height * 4, 0.0f);
    vars.reCreate<ge::gl::Texture>("computeShaderImage", GL_TEXTURE_2D, GL_RGBA32F, 0, width, height)->clear(0, GL_RGBA, GL_FLOAT, &emptyData[0]);
    vars.get<ge::gl::Texture>("computeShaderImage")->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    vars.get<ge::gl::Texture>("computeShaderImage")->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    vars.get<ge::gl::Texture>("computeShaderImage")->bindImage(1, 0, GL_RGBA32F, GL_READ_WRITE);
    vars.get<ge::gl::Texture>("computeShaderImage")->bind(0);
}

void RayTracedChess::draw()
{
    static auto computeProgram = vars.get<ge::gl::Program>("computeProgram");
    static auto basicDrawProgram = vars.get<ge::gl::Program>("basicDrawProgram");
    static auto backgroundVAO = vars.get<ge::gl::VertexArray>("backgroundVAO");
    static auto workGroupSizeXInv = 1.0 / static_cast<float>(vars.getInt32("workGroupSizeX"));
    static auto workGroupSizeYInv = 1.0 / static_cast<float>(vars.getInt32("workGroupSizeY"));
    camera.update(deltaTime * speedMultiplier);
    computeProgram->set("camera.direction", camera.getDirection().x, camera.getDirection().y, camera.getDirection().z, 0.0f);
    computeProgram->set("camera.up", camera.getUp().x, camera.getUp().y, camera.getUp().z, 0.0f);
    computeProgram->set("camera.left", camera.getLeft().x, camera.getLeft().y, camera.getLeft().z, 0.0f);
    computeProgram->set("camera.position", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z, 1.0f);
    computeProgram->set("camera.sensorHalfWidth", camera.getSensorHalfWidth());
    computeProgram->set("globalSeed", (float)glfwGetTime());
    computeProgram->set("colorMultiplier", resetTexture ? 0.0f : 1.0f);
    resetTexture = false;
    computeProgram->dispatch((int)ceil(width * workGroupSizeXInv), (int)ceil(height * workGroupSizeYInv), 1);
    ge::gl::glFinish();

    backgroundVAO->bind();
    basicDrawProgram->use();
    ge::gl::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    backgroundVAO->unbind();

    drawGui(drawGuiB);
    swap();
}

void RayTracedChess::mouseButtonEvent(int button, int action, int mods)
{}

void RayTracedChess::mouseScrollEvent(double xoffset, double yoffset)
{
    if (yoffset > 0)
        speedMultiplier *= 1.5;
    if (yoffset < 0)
        speedMultiplier /= 1.5;
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

    float xoffset = (float)(xpos - lastX);
    float yoffset = (float)(lastY - ypos);
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
    resetTexture = true;
}

void RayTracedChess::keyEvent(int key, int scancode, int action, int mods)
{
    static int moveMultiplier = 20;
    bool pressed = action == GLFW_PRESS;
    bool repeat = action == GLFW_REPEAT;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        drawGuiB = !drawGuiB;
        glfwSetInputMode(window, GLFW_CURSOR, drawGuiB ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        return;
    }
    if (key == GLFW_KEY_W && !repeat)
        camera.moveFront(pressed);
    if (key == GLFW_KEY_A && !repeat)
        camera.moveLeft(pressed);
    if (key == GLFW_KEY_S && !repeat)
        camera.moveBack(pressed);
    if (key == GLFW_KEY_D && !repeat)
        camera.moveRight(pressed);
    resetTexture = true;
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
