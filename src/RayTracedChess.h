#pragma once
#include "Application.h"
#include "Vars/Vars.h"
#include <string>
#include "Chess/Chess.h"
#include "Utils/hdrloader.h"
#include "RenderInfo.h"

class RayTracedChess : public Application {
public:
    RayTracedChess();

    void draw() override;
    void mouseButtonEvent(int button, int action, int mods) override;
    void mouseScrollEvent(double xoffset, double yoffset) override;
    void mouseMoveEvent(double xpos, double ypos) override;
    void keyEvent(int key, int scancode, int action, int mods) override;
    void resizeEvent(int width, int height) override;
    void updateBuffers();

private:
    vars::Vars vars;

    void checkKeys();
    void initComputeShaderImage();
    void saveRenderToFile();
    void drawGui(bool drawGui);

    bool drawGuiB = true;
    bool resetRender = true;
    bool saveImage = true;
    Chess::Chess scene;
    //Scene scene;
    RenderInfo renderInfo;
    HDRLoaderResult backgroundTexture;
    double firstMouse = true;
    const int pathAlive = 1 << 19;
    GLint workGroupSizeRes[3];
    std::unique_ptr<ge::gl::Program> resetProgram;
    GLint workGroupSizeLog[3];
    std::unique_ptr<ge::gl::Program> logicProgram;
    GLint workGroupSizeNew[3];
    std::unique_ptr<ge::gl::Program> newPathProgram;
    GLint workGroupSizeMat[3];
    std::unique_ptr<ge::gl::Program> basicMaterialProgram;
    GLint workGroupSizeExt[3];
    std::unique_ptr<ge::gl::Program> extRayProgram;
    std::unique_ptr<ge::gl::Program> shadRayProgram;
    std::unique_ptr<ge::gl::Program> basicDrawProgram;
};
