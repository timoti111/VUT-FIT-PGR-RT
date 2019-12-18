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
#include "misc/cpp/imgui_stdlib.h"
#include "Scene/Geometry/Textures.h"
#include "lodepng.h"

RayTracedChess::RayTracedChess() : Application(), scene("res/models/chess/board2/", "res/models/chess/set1/")
{
    std::cout << "Compiling reset shader" << std::endl;
    resetProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/stageResetRender.glsl")));
    std::cout << "Compiling logic shader" << std::endl;
    logicProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/stageLogic.glsl")));
    std::cout << "Compiling newPath shader" << std::endl;
    newPathProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/stageNewPath.glsl")));
    std::cout << "Compiling extRay shader" << std::endl;
    extRayProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/stageExtensionRayCast.glsl")));
    std::cout << "Compiling shadRay shader" << std::endl;
    shadRayProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/stageShadowRayCast.glsl")));
    std::cout << "Compiling material shader" << std::endl;
    basicMaterialProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER, Shadinclude::load("res/shaders/stageMaterial.glsl")));
    std::cout << "Compiling draw shader" << std::endl;
    basicDrawProgram = std::make_unique<ge::gl::Program>(
        std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER, Shadinclude::load("res/shaders/basicVertexShader.glsl")),
        std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER, Shadinclude::load("res/shaders/basicFragmentShader.glsl")));
    float data[] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f };
    vars.add<ge::gl::Buffer>("backgroundQuad", sizeof(float) * 8, data);
    vars.add<ge::gl::VertexArray>("backgroundVAO")->addAttrib(
        vars.get<ge::gl::Buffer>("backgroundQuad"), 0, 2, GL_FLOAT);
    vars.get<ge::gl::VertexArray>("backgroundVAO")->bind();

    HDRLoader::load("res/textures/delta_2_4k.hdr", backgroundTexture);
    auto texture = std::make_shared<ge::gl::Texture>(GL_TEXTURE_2D, GL_RGB32F, 10, backgroundTexture.width, backgroundTexture.height);
    texture->setData2D(backgroundTexture.cols.data(), GL_RGB, GL_FLOAT);
    texture->generateMipmap();
    texture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    texture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    renderInfo.renderParams.environmentMapTextureID = Textures::addTexture(texture);

    initComputeShaderImage();

    vars.reCreate<ge::gl::Buffer>("pathsDataBuffer", (4 + 1 + 4 * pathAlive + 64 * pathAlive) * sizeof(unsigned) + 3)->bindBase(GL_SHADER_STORAGE_BUFFER, logicProgram->getBufferBinding("PathsDataBuffer"));
    vars.reCreate<ge::gl::Buffer>("renderParamsBuffer", sizeof(RenderParameters))->bindBase(GL_UNIFORM_BUFFER, 1);
    resetProgram->getComputeWorkGroupSize(workGroupSizeRes);
    logicProgram->getComputeWorkGroupSize(workGroupSizeLog);
    newPathProgram->getComputeWorkGroupSize(workGroupSizeNew);
    basicMaterialProgram->getComputeWorkGroupSize(workGroupSizeMat);
    extRayProgram->getComputeWorkGroupSize(workGroupSizeExt);
    shadRayProgram->getComputeWorkGroupSize(workGroupSizeSha);

    scene.setSceneUpdateCallback([=](bool materials)
    {
        if (materials)
        {
            vars.reCreate<ge::gl::Buffer>("materialsBuffer", Material::getMaterials())->bindBase(GL_SHADER_STORAGE_BUFFER, logicProgram->getBufferBinding("MaterialsBuffer"));
            renderInfo.renderParams.numberOfLights = scene.lights.lights.size();
            vars.reCreate<ge::gl::Buffer>("lightsBuffer", scene.lights.lights, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, logicProgram->getBufferBinding("LightsBuffer"));
        }
        else
            updateBuffers();
        resetRender = true;
    });

    renderInfo.setParamsUpdatedCallback([=]()
    {
        resetRender = true;
    });

    scene.updateBVHs();
    updateBuffers();
    Textures::bindTextures(3);
}

void RayTracedChess::initComputeShaderImage()
{
    static std::vector<GLfloat> emptyData(width * height * 4, 0.0f);
    vars.reCreate<ge::gl::Texture>("computeShaderImage", GL_TEXTURE_2D, GL_RGBA32F, 0, width, height)->clear(0, GL_RGBA, GL_FLOAT, &emptyData[0]);
    vars.get<ge::gl::Texture>("computeShaderImage")->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    vars.get<ge::gl::Texture>("computeShaderImage")->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    vars.get<ge::gl::Texture>("computeShaderImage")->bindImage(2, 0, GL_RGBA32F, GL_READ_WRITE);
    vars.get<ge::gl::Texture>("computeShaderImage")->bind(0);
}

void RayTracedChess::draw()
{
    static float WGSizeInvRes = 1.0f / workGroupSizeRes[0];
    static float WGSizeInvLog = 1.0f / workGroupSizeLog[0];
    static float WGSizeInvNew = 1.0f / workGroupSizeNew[0];
    static float WGSizeInvMat = 1.0f / workGroupSizeMat[0];
    static float WGSizeInvExt = 1.0f / workGroupSizeExt[0];
    static float WGSizeInvSha = 1.0f / workGroupSizeSha[0];
    static auto renderParamsBuffer = vars.get<ge::gl::Buffer>("renderParamsBuffer");
    static unsigned iteration = 0;
    static RenderParameters actualRenderParameters;

    if (!drawGuiB)
        checkKeys();

    if (resetRender)
    {
        actualRenderParameters = renderInfo.renderParams;
        renderParamsBuffer->setData(&actualRenderParameters, sizeof(RenderParameters));
        resetRender = false;
        iteration = 0;
    }

    int advanceBy = 1;

    if (iteration < 2)
    {
        advanceBy = (int)std::ceil((2 * width * height) / (float)pathAlive);

        resetProgram->dispatch((int)ceil(std::max(pathAlive, width * height) * WGSizeInvRes), 1, 1);
        newPathProgram->dispatch((int)ceil(pathAlive * WGSizeInvNew), 1, 1);
        extRayProgram->dispatch((int)ceil(pathAlive * WGSizeInvExt), 1, 1);
    }

    for (int i = 0; i < advanceBy; i++)
    {
        logicProgram->dispatch((int)ceil(pathAlive * WGSizeInvLog), 1, 1);
        newPathProgram->dispatch((int)ceil(pathAlive * WGSizeInvNew), 1, 1);
        basicMaterialProgram->dispatch((int)ceil(pathAlive * WGSizeInvMat), 1, 1);
        extRayProgram->dispatch((int)ceil(pathAlive * WGSizeInvExt), 1, 1);
        shadRayProgram->dispatch((int)ceil(pathAlive * WGSizeInvSha), 1, 1);
    }

    basicDrawProgram->use();
    ge::gl::glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (saveImage)
        saveRenderToFile();

    drawGui(drawGuiB);
    swap();
    iteration++;
}

void RayTracedChess::mouseButtonEvent(int button, int action, int mods)
{
    if (ImGui::IsAnyWindowFocused())
        return;
    if (mods == 0 && action == GLFW_RELEASE && drawGuiB)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        Ray ray = Ray::createCameraRay(&renderInfo, glm::vec2(xpos, height - ypos), glm::ivec2(width, height));
        scene.selectMesh(ray);
    }
    if (mods == GLFW_MOD_CONTROL && action == GLFW_RELEASE && drawGuiB)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        Ray ray = Ray::createCameraRay(&renderInfo, glm::vec2(xpos, height - ypos), glm::ivec2(width, height));
        ray.traceRay(&scene);
        if (ray.t != std::numeric_limits<float>::max())
            renderInfo.setFocusDistance(ray.t);
    }
}

void RayTracedChess::mouseMoveEvent(double xpos, double ypos)
{
    static glm::vec2 lastPos;
    if (drawGuiB)
        return;
    ypos = -ypos;
    if (firstMouse)
    {
        lastPos.x = xpos;
        lastPos.y = ypos;
        firstMouse = false;
    }

    auto newPos = glm::vec2(xpos, ypos);
    renderInfo.rotateCamera(newPos - lastPos);
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
    resetRender = true;
}

void RayTracedChess::updateBuffers()
{
    vars.reCreate<ge::gl::Buffer>("materialsBuffer", Material::getMaterials())->bindBase(GL_SHADER_STORAGE_BUFFER, logicProgram->getBufferBinding("MaterialsBuffer"));
    vars.reCreate<ge::gl::Buffer>("sceneBVHBuffer", scene.getFlatTree(), GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, logicProgram->getBufferBinding("SceneBVHBuffer"));
    vars.reCreate<ge::gl::Buffer>("meshBHVs", scene.meshBVHs, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, logicProgram->getBufferBinding("MeshBVHsBuffer"));
    vars.reCreate<ge::gl::Buffer>("meshesBuffer", scene.meshesGPU, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, logicProgram->getBufferBinding("MeshesBuffer"));
    vars.reCreate<ge::gl::Buffer>("primitivesBuffer", scene.primitivesGPU, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, logicProgram->getBufferBinding("PrimitivesBuffer"));
    vars.reCreate<ge::gl::Buffer>("trianglesBuffer", scene.triangles, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, logicProgram->getBufferBinding("TrianglesBuffer"));
    vars.reCreate<ge::gl::Buffer>("spheresBuffer", scene.spheres, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, logicProgram->getBufferBinding("SpheresBuffer"));
    vars.reCreate<ge::gl::Buffer>("cylindersBuffer", scene.cylinders, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, logicProgram->getBufferBinding("CylindersBuffer"));
    vars.reCreate<ge::gl::Buffer>("lightsBuffer", scene.lights.lights, GL_STATIC_READ)->bindBase(GL_SHADER_STORAGE_BUFFER, logicProgram->getBufferBinding("LightsBuffer"));
}

static std::string rednerFilename;
void RayTracedChess::saveRenderToFile()
{
    if (rednerFilename.empty())
        return;
    auto renderData = std::vector<unsigned char>(width * height * 4, 0);
    ge::gl::glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, renderData.data());
    for (int i = 0; i < (height / 2); i++)
        std::swap_ranges(renderData.begin() + i * width * 4,
                         renderData.begin() + (i + 1) * width * 4,
                         renderData.end() - (i + 1) * width * 4);
    lodepng::encode(rednerFilename + ".png", renderData, width, height);
    saveImage = false;
}

void RayTracedChess::drawGui(bool drawGui)
{
    if (!drawGui)
        return;

    ImGui::Begin("App info");
    ImGui::InputText("Filename", &rednerFilename);
    if (ImGui::Button("Save render"))
        saveImage = true;
    if (ImGui::Button("Add Light"))
        scene.addLight();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    if (ImGui::CollapsingHeader("Help"))
    {
        ImGui::TextWrapped(R"(Controls:

When GUI is turned on:
Left click - select object
Ctrl + Left click - focus on object
G - turn off GUI

When GUI is turned off:
WASD - Movement
G - turn on GUI
)");
    }
    ImGui::End();

    renderInfo.drawGui();
    scene.drawGui();
}
