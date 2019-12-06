#include "RayTracedChess.h"
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
#include "RayTracing/Ray.h"
#include "Utils/Shadinclude.hpp"

RayTracedChess::RayTracedChess() : Application(), scene("res/models/chess/board2/", "res/models/chess/set1/")
{
    //vars.add<ge::gl::Program>("computeProgram",
    //                          std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/computeShader.glsl")));
    vars.add<ge::gl::Program>("logicProgram",
                              std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/logic.glsl")));
    vars.add<ge::gl::Program>("newPathProgram",
                              std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/newPath.glsl")));
    vars.add<ge::gl::Program>("extRayProgram",
                              std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/extensionRayCast.glsl")));
    vars.add<ge::gl::Program>("basicMaterialProgram",
                              std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/basicMaterial.glsl")));
    vars.add<ge::gl::Program>("basicDrawProgram",
                              std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER, Shadinclude::load("res/shaders/basicVertexShader.glsl")),
                              std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER, Shadinclude::load("res/shaders/basicFragmentShader.glsl")));
    ;
    float data[] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f };
    vars.add<ge::gl::Buffer>("backgroundQuad", sizeof(float) * 8, data);
    vars.add<ge::gl::VertexArray>("backgroundVAO")->addAttrib(
        vars.get<ge::gl::Buffer>("backgroundQuad"), 0, 2, GL_FLOAT);
    vars.get<ge::gl::VertexArray>("backgroundVAO")->bind();

    HDRLoader::load("res/textures/delta_2_4k.hdr", backgroundTexture);
    vars.add<ge::gl::Texture>("enviromentTexture", GL_TEXTURE_2D, GL_RGB32F, 10, backgroundTexture.width, backgroundTexture.height)
        ->setData2D(backgroundTexture.cols.data(), GL_RGB, GL_FLOAT);
    vars.get<ge::gl::Texture>("enviromentTexture")->generateMipmap();
    vars.get<ge::gl::Texture>("enviromentTexture")->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    vars.get<ge::gl::Texture>("enviromentTexture")->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    vars.get<ge::gl::Texture>("enviromentTexture")->bind(2);

    initComputeShaderImage();

    static auto emptyCounters = std::vector<unsigned>(5, 0);
    static auto emptyPaths = std::vector<float>(pathAlive * 40, 0.0f);
    static auto emptyPathIndices = std::vector<int>(pathAlive, 0);
    vars.reCreate<ge::gl::Buffer>("atomicCountersBuffer", emptyCounters, GL_DYNAMIC_DRAW)->bindBase(GL_ATOMIC_COUNTER_BUFFER, 0);
    vars.reCreate<ge::gl::Buffer>("pathsBuffer", emptyPaths, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 1);
    vars.reCreate<ge::gl::Buffer>("newPathBuffer", emptyPathIndices, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 2);
    vars.reCreate<ge::gl::Buffer>("extRayCastBuffer", emptyPathIndices, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 3);
    vars.reCreate<ge::gl::Buffer>("shadowRayCastBuffer", emptyPathIndices, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 4);
    vars.reCreate<ge::gl::Buffer>("basicMaterialBuffer", emptyPathIndices, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 5);

    GLint workGroupSize[3];
    //ge::gl::glGetProgramiv(vars.get<ge::gl::Program>("computeProgram")->getId(), GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
    ge::gl::glGetProgramiv(vars.get<ge::gl::Program>("logicProgram")->getId(), GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
    vars.addInt32("workGroupSizeX", workGroupSize[0]);
    vars.addInt32("workGroupSizeY", workGroupSize[1]);
    vars.addInt32("workGroupSizeZ", workGroupSize[2]);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    scene.setSceneUpdateCallback([=]()
    {
        updateBuffers();
        resetTexture = true;
    });

    auto ident = glm::mat4x4(1.0f);
    double actTime = glfwGetTime();
    //scene.addShape(Geometry::Shape::fromObjFile("res/models/chess/set1/Knight.obj", "Knight"));
    //scene.addShape(Geometry::Shape::fromObjFile("res/models/chess/board1/Board.obj", "Board"));
    //double time = glfwGetTime() - actTime;
    //std::cout << "Model load time: " << time << std::endl;
    //actTime = glfwGetTime();
    //scene.instantiateShape("Board", ident);
    //time = glfwGetTime() - actTime;
    //std::cout << "BVH build time: " << time << std::endl;
    //scene.instantiateShape("Knight", glm::translate(ident, glm::vec3(-3.514, 0.80321, -3.514)));

    //scene.addShape(Geometry::Shape::fromObjFile("res/models/chess/set1/Chess.obj", "Sponza"));
    //scene.instantiateShape("Sponza", ident);

    scene.updateBVHs();
    updateBuffers();
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
    //static auto computeProgram = vars.get<ge::gl::Program>("computeProgram");
    //static auto basicDrawProgram = vars.get<ge::gl::Program>("basicDrawProgram");
    //static auto backgroundVAO = vars.get<ge::gl::VertexArray>("backgroundVAO");
    static auto workGroupSizeXInv = 1.0 / static_cast<float>(vars.getInt32("workGroupSizeX"));
    static auto workGroupSizeYInv = 1.0 / static_cast<float>(vars.getInt32("workGroupSizeY"));
    camera.update(deltaTime * speedMultiplier);
    //computeProgram->set("camera.direction", camera.getDirection().x, camera.getDirection().y, camera.getDirection().z, 0.0f);
    //computeProgram->set("camera.up", camera.getUp().x, camera.getUp().y, camera.getUp().z, 0.0f);
    //computeProgram->set("camera.left", camera.getLeft().x, camera.getLeft().y, camera.getLeft().z, 0.0f);
    //computeProgram->set("camera.position", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z, 1.0f);
    //computeProgram->set("camera.sensorHalfWidth", camera.getSensorHalfWidth());
    //computeProgram->set("globalSeed", (float)glfwGetTime());
    //computeProgram->set("colorMultiplier", resetTexture ? 0.0f : 1.0f);
    //resetTexture = false;
    //computeProgram->dispatch((int)ceil(width * workGroupSizeXInv), (int)ceil(height * workGroupSizeYInv), 1);
    //ge::gl::glFinish();

    //static auto rayGenProgram = vars.get<ge::gl::Program>("rayGenProgram");
    //static auto extendProgram = vars.get<ge::gl::Program>("extendProgram");
    //static auto basicDrawProgram = vars.get<ge::gl::Program>("basicDrawProgram");
    //static auto backgroundVAO = vars.get<ge::gl::VertexArray>("backgroundVAO");
    //static auto workGroupSizeXInv = 1.0 / static_cast<float>(vars.getInt32("workGroupSizeX"));
    //static auto workGroupSizeYInv = 1.0 / static_cast<float>(vars.getInt32("workGroupSizeY"));
    //camera.update(deltaTime * speedMultiplier);
    //rayGenProgram->set("camera.direction", camera.getDirection().x, camera.getDirection().y, camera.getDirection().z, 0.0f);
    //rayGenProgram->set("camera.up", camera.getUp().x, camera.getUp().y, camera.getUp().z, 0.0f);
    //rayGenProgram->set("camera.left", camera.getLeft().x, camera.getLeft().y, camera.getLeft().z, 0.0f);
    //rayGenProgram->set("camera.position", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z, 1.0f);
    //rayGenProgram->set("camera.sensorHalfWidth", camera.getSensorHalfWidth());

    //rayGenProgram->set("resolution", width, height);
    //computeProgram->set("colorMultiplier", resetTexture ? 0.0f : 1.0f);
    //resetTexture = false;
    //rayGenProgram->dispatch((int)ceil(width * workGroupSizeXInv), (int)ceil(height * workGroupSizeYInv), 1);
    //ge::gl::glFinish();
    //extendProgram->dispatch((int)ceil(width * height) / 64, 1, 1);
    //ge::gl::glFinish();

    //static auto atomicCounters = vars.get<ge::gl::Buffer>("atomicCountersBuffer");
    //static GLuint* data = static_cast<GLuint*>(atomicCounters->map());
    //GLuint* data = static_cast<GLuint*>(atomicCounters->map());
    //data[1] = 0;
    //atomicCounters->unmap();

    static float actualTime = 0;
    std::cout << "Other time: " << glfwGetTime() - actualTime << std::endl;

    actualTime = glfwGetTime();
    static auto logicProgram = vars.get<ge::gl::Program>("logicProgram");
    logicProgram->set("colorMultiplier", resetTexture ? 0.0f : 1.0f);
    resetTexture = false;
    logicProgram->dispatch((int)ceil(pathAlive * workGroupSizeXInv), 1, 1);
    ge::gl::glFinish();
    std::cout << "Logic Program time: " << glfwGetTime() - actualTime << std::endl;

    actualTime = glfwGetTime();
    static auto newPathProgram = vars.get<ge::gl::Program>("newPathProgram");
    newPathProgram->set("globalSeed", (float)glfwGetTime());
    newPathProgram->set("camera.direction", camera.getDirection().x, camera.getDirection().y, camera.getDirection().z, 0.0f);
    newPathProgram->set("camera.up", camera.getUp().x, camera.getUp().y, camera.getUp().z, 0.0f);
    newPathProgram->set("camera.left", camera.getLeft().x, camera.getLeft().y, camera.getLeft().z, 0.0f);
    newPathProgram->set("camera.position", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z, 1.0f);
    newPathProgram->set("camera.sensorHalfWidth", camera.getSensorHalfWidth());
    newPathProgram->dispatch((int)ceil(pathAlive * workGroupSizeXInv), 1, 1);
    ge::gl::glFinish();
    std::cout << "NewPath Program time: " << glfwGetTime() - actualTime << std::endl;

    actualTime = glfwGetTime();
    static auto extRayProgram = vars.get<ge::gl::Program>("extRayProgram");
    extRayProgram->dispatch((int)ceil(pathAlive * workGroupSizeXInv), 1, 1);
    ge::gl::glFinish();
    std::cout << "Extension Program time: " << glfwGetTime() - actualTime << std::endl;

    actualTime = glfwGetTime();
    static auto basicMaterialProgram = vars.get<ge::gl::Program>("basicMaterialProgram");
    basicMaterialProgram->dispatch((int)ceil(pathAlive * workGroupSizeXInv), 1, 1);
    ge::gl::glFinish();
    std::cout << "Material Program time: " << glfwGetTime() - actualTime << std::endl;

    static auto basicDrawProgram = vars.get<ge::gl::Program>("basicDrawProgram");
    basicDrawProgram->use();
    ge::gl::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    actualTime = glfwGetTime();
    drawGui(drawGuiB);
    std::cout << "Draw GUI time: " << glfwGetTime() - actualTime << std::endl;
    actualTime = glfwGetTime();
    swap();
    std::cout << "Swap time: " << glfwGetTime() - actualTime << std::endl;
    actualTime = glfwGetTime();
}

void RayTracedChess::mouseButtonEvent(int button, int action, int mods)
{
    if (ImGui::IsAnyWindowFocused())
        return;
    if (action == GLFW_RELEASE && drawGuiB)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        Ray ray = Ray::createCameraRay(&camera, glm::vec2(xpos, height - ypos), glm::ivec2(width, height));
        scene.selectMesh(ray);
    }
}

void RayTracedChess::mouseScrollEvent(double xoffset, double yoffset)
{
    if (drawGuiB)
        return;

    if (yoffset > 0)
        speedMultiplier *= 1.5;
    if (yoffset < 0)
        speedMultiplier /= 1.5;
}

void RayTracedChess::mouseMoveEvent(double xpos, double ypos)
{
    if (drawGuiB)
        return;

    ypos = -ypos;
    static double lastX, lastY;
    static float yaw = 0.0f, pitch = 0.0f;

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
        firstMouse = true;
        return;
    }

    if (drawGuiB)
        return;

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

void RayTracedChess::updateBuffers()
{
    vars.reCreate<ge::gl::Buffer>("sceneBVHBuffer", scene.getFlatTree(), GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 6);
    vars.reCreate<ge::gl::Buffer>("meshBHVs", scene.meshBVHs, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 7);
    vars.reCreate<ge::gl::Buffer>("meshesBuffer", scene.meshesGPU, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 8);
    vars.reCreate<ge::gl::Buffer>("primitivesBuffer", scene.primitivesGPU, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 9);
    vars.reCreate<ge::gl::Buffer>("trianglesBuffer", scene.triangles, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 10);
    vars.reCreate<ge::gl::Buffer>("verticesBuffer", scene.vertices, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 11);
    vars.reCreate<ge::gl::Buffer>("normalsBuffer", scene.normals, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 12);
    vars.reCreate<ge::gl::Buffer>("coordsBuffer", scene.coords, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 13);
    vars.reCreate<ge::gl::Buffer>("spheresBuffer", scene.spheres, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 14);
    vars.reCreate<ge::gl::Buffer>("cylindersBuffer", scene.cylinders, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 15);
}

void RayTracedChess::drawGui(bool drawGui)
{
    if (!drawGui)
        return;

    ImGui::Begin("App info");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    camera.drawImGui();
    scene.drawSelectedPieceSettings();
}
