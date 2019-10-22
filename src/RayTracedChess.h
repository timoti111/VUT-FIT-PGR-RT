#pragma once
#include "Application.h"
#include "Vars/Vars.h"
#include <string>
#include "RayTracing/Camera.h"
#include "RayTracing/Scene.h"
#include "Utils/hdrloader.h"

class RayTracedChess : public Application {
public:
    RayTracedChess();

    void draw() override;
    void mouseButtonEvent(int button, int action, int mods) override;
    void mouseScrollEvent(double xoffset, double yoffset) override;
    void mouseMoveEvent(double xpos, double ypos) override;
    void keyEvent(int key, int scancode, int action, int mods) override;
    void resizeEvent(int width, int height) override;

private:
    vars::Vars vars;

    void initComputeShaderImage();
    void drawGui(bool drawGui);
    double speedMultiplier = 1.0;

    bool drawGuiB = false;
    Camera camera;
    HDRLoaderResult backgroundTexture;
};
