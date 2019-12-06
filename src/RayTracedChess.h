﻿#pragma once
#include "Application.h"
#include "Vars/Vars.h"
#include <string>
#include "Scene/Camera.h"
#include "Chess/Chess.h"
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
    void updateBuffers();

private:
    vars::Vars vars;

    void initComputeShaderImage();
    void drawGui(bool drawGui);
    double speedMultiplier = 1.0;

    bool drawGuiB = true;
    bool resetTexture = false;
    Camera camera;
    Chess::Chess scene;
    HDRLoaderResult backgroundTexture;
    double firstMouse = true;
    static const int pathAlive = 500000;
};
