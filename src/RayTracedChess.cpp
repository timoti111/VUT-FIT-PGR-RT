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
#include <algorithm>

RayTracedChess::RayTracedChess() : Application(), scene("res/models/chess/board2/", "res/models/chess/set1/")
{
    //vars.add<ge::gl::Program>("computeProgram",
    //                          std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/computeShader.glsl")));
    resetProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/reset.glsl")));
    logicProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/logic.glsl")));
    newPathProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/newPath.glsl")));
    extRayProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/extensionRayCast.glsl")));
    shadRayProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/shadowRayCast.glsl")));
    basicMaterialProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/basicMaterial.glsl")));
    basicDrawProgram = std::make_unique<ge::gl::Program>(
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

    static auto emptyCounters = std::vector<unsigned>(4, 0);
    static auto emptyPaths = std::vector<float>(pathAlive * 52, 0.0f);
    static auto emptyPathIndices = std::vector<int>(pathAlive, 0);
    vars.reCreate<ge::gl::Buffer>("renderParamsBuffer", sizeof(RenderParameters))->bindBase(GL_UNIFORM_BUFFER, 0);
    vars.reCreate<ge::gl::Buffer>("atomicCountersBuffer", emptyCounters, GL_DYNAMIC_DRAW)->bindBase(GL_SHADER_STORAGE_BUFFER, 0);
    vars.reCreate<ge::gl::Buffer>("pathsBuffer", emptyPaths, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 1);
    vars.reCreate<ge::gl::Buffer>("newPathBuffer", emptyPathIndices, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 2);
    vars.reCreate<ge::gl::Buffer>("extRayCastBuffer", emptyPathIndices, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 3);
    vars.reCreate<ge::gl::Buffer>("shadowRayCastBuffer", emptyPathIndices, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 4);
    vars.reCreate<ge::gl::Buffer>("basicMaterialBuffer", emptyPathIndices, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, 5);

    GLint workGroupSize[3];
    ge::gl::glGetProgramiv(logicProgram->getId(), GL_COMPUTE_WORK_GROUP_SIZE, workGroupSize);
    vars.addInt32("workGroupSizeX", workGroupSize[0]);
    vars.addInt32("workGroupSizeY", workGroupSize[1]);
    vars.addInt32("workGroupSizeZ", workGroupSize[2]);

    scene.setSceneUpdateCallback([=]()
    {
        updateBuffers();
        updateParams = true;
    });

    renderInfo.setParamsUpdatedCallback([=]()
    {
        updateParams = true;
    });

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
    static auto workGroupSizeXInv = 1.0 / static_cast<float>(vars.getInt32("workGroupSizeX"));
    static auto workGroupSizeYInv = 1.0 / static_cast<float>(vars.getInt32("workGroupSizeY"));
    checkKeys();

    static auto renderParamsBuffer = vars.get<ge::gl::Buffer>("renderParamsBuffer");
    static auto atomicCountersBuffer = vars.get<ge::gl::Buffer>("atomicCountersBuffer");
    static auto zeroCounters = std::vector<unsigned>(4, 0);
    static auto readCounters = std::vector<unsigned>(4, 0);
    static unsigned iteration = 0;
    static GLuint newPaths = 0;
    static GLuint currentPixelIndex = 0;
    static RenderParameters actualRenderParameters;

    if (updateParams)
    {
        actualRenderParameters = renderInfo.renderParams;
        actualRenderParameters.maxBounces = renderInfo.getPreviewBounces();
        renderParamsBuffer->setData(&actualRenderParameters, sizeof(RenderParameters));
        updateParams = false;
        iteration = 0;
        newPaths = 0;
    }

    int advanceBy = 1;

    if (iteration < 2)
    {
        currentPixelIndex = 0;
        if (iteration == 0)
            advanceBy = std::numeric_limits<int>::max();

        resetProgram->set("firstIteration", GLuint(iteration == 0));
        resetProgram->dispatch((int)ceil(std::max(pathAlive, width * height) * workGroupSizeXInv), 1, 1);
        newPathProgram->set("newPixelIndex", currentPixelIndex);
        atomicCountersBuffer->getData(&newPaths, sizeof(newPaths));
        currentPixelIndex += newPaths;
        newPathProgram->dispatch((int)ceil(pathAlive * workGroupSizeXInv), 1, 1);
        extRayProgram->dispatch((int)ceil(pathAlive * workGroupSizeXInv), 1, 1);
        atomicCountersBuffer->setData(zeroCounters);
    }

    for (int i = 0; i < advanceBy; i++)
    {
        newPathProgram->set("newPixelIndex", currentPixelIndex);
        logicProgram->set("currentPixelCount", currentPixelIndex);
        logicProgram->set("firstIteration", GLuint(iteration == 0));
        logicProgram->dispatch((int)ceil(pathAlive * workGroupSizeXInv), 1, 1);
        if (iteration == 0)
        {
            atomicCountersBuffer->getData(readCounters);
            if (readCounters == zeroCounters)
                break;
            newPaths = readCounters[0];
        }
        else
            atomicCountersBuffer->getData(&newPaths, sizeof(newPaths));
        currentPixelIndex += newPaths;
        newPathProgram->set("globalSeed", (float)glfwGetTime());
        newPathProgram->dispatch((int)ceil(pathAlive * workGroupSizeXInv), 1, 1);
        basicMaterialProgram->set("firstIteration", GLuint(iteration == 0));
        basicMaterialProgram->dispatch((int)ceil(pathAlive * workGroupSizeXInv), 1, 1);
        extRayProgram->dispatch((int)ceil(pathAlive * workGroupSizeXInv), 1, 1);
        //shadRayProgram->dispatch((int)ceil(pathAlive * workGroupSizeXInv), 1, 1);
        atomicCountersBuffer->setData(zeroCounters);
    }

    if (iteration == 0)
    {
        actualRenderParameters.maxBounces = renderInfo.renderParams.maxBounces;
        renderParamsBuffer->setData(&actualRenderParameters, sizeof(RenderParameters));
        //renderParamsBuffer->setData(&actualRenderParameters.maxBounces,
        //                            sizeof(actualRenderParameters.maxBounces),
        //                            (char*)(&actualRenderParameters.maxBounces) - (char*)(&actualRenderParameters));
    }

    basicDrawProgram->use();
    ge::gl::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    drawGui(drawGuiB);
    swap();
    iteration++;
}

void RayTracedChess::mouseButtonEvent(int button, int action, int mods)
{
    if (ImGui::IsAnyWindowFocused())
        return;
    if (action == GLFW_RELEASE && drawGuiB)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        Ray ray = Ray::createCameraRay(&renderInfo, glm::vec2(xpos, height - ypos), glm::ivec2(width, height));
        scene.selectMesh(ray);
    }
}

void RayTracedChess::mouseScrollEvent(double xoffset, double yoffset)
{
    if (drawGuiB)
        return;

    //if (yoffset > 0)
    //    speedMultiplier *= 1.5;
    //if (yoffset < 0)
    //    speedMultiplier /= 1.5;
}

void RayTracedChess::mouseMoveEvent(double xpos, double ypos)
{
    if (drawGuiB)
        return;
    ypos = -ypos;
    static glm::vec2 lastPos;
    if (firstMouse)
    {
        lastPos.x = xpos;
        lastPos.y = ypos;
        firstMouse = false;
    }

    auto newPos = glm::vec2(xpos, ypos);
    auto delta = newPos - lastPos;
    renderInfo.rotateCamera(delta);
    lastPos = newPos;
}

void RayTracedChess::keyEvent(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        drawGuiB = !drawGuiB;
        glfwSetInputMode(window, GLFW_CURSOR, drawGuiB ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        firstMouse = true;
        return;
    }
}

#define checkAndDo(key, todo) if(glfwGetKey(window, key) == GLFW_PRESS) { todo; }
void RayTracedChess::checkKeys()
{
    checkAndDo(GLFW_KEY_W, renderInfo.moveFront(deltaTime));
    checkAndDo(GLFW_KEY_A, renderInfo.moveLeft(deltaTime));
    checkAndDo(GLFW_KEY_S, renderInfo.moveBack(deltaTime));
    checkAndDo(GLFW_KEY_D, renderInfo.moveRight(deltaTime));
}

void RayTracedChess::resizeEvent(int x, int y)
{
    initComputeShaderImage();
    updateParams = true;
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

    renderInfo.drawImGui();
    //scene.drawSelectedPieceSettings();
}
