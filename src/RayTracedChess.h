#pragma once
#include "Application.h"
#include "Vars/Vars.h"
#include "imgui.h"
#include <string>
#include "RayTracing/Camera.h"
#include "Utils/hdrloader.h"
//#include <geGL/geGL.h>

class RayTracedChess : public Application {
public:
    RayTracedChess();

    void draw() override;
    void mouseButtonEvent(int button, int action, int mods) override;
    void mouseMoveEvent(double xpos, double ypos) override;
    void keyEvent(int key, int scancode, int action, int mods) override;
    void resizeEvent(int width, int height) override;

private:
    vars::Vars vars;

    void initComputeShaderImage();
    void drawGui(bool drawGui);
    int workGroupSizeX = 0, workGroupSizeY = 0, workGroupSizeZ = 0;

    bool drawGuiB = false;
    Camera camera;
    HDRLoaderResult backgroundTexture;
};
