#pragma once

#include <functional>
#include <geGL/geGL.h>
#include "GLFW/glfw3.h"

class Application {
public:
    Application();
    virtual ~Application() {};
    void start();
    double getDeltaTime();
    void swap();
    virtual void mouseScrollEvent(double xoffset, double yoffset) {};
    virtual void mouseMoveEvent(double xpos, double ypos) {};
    virtual void mouseButtonEvent(int button, int action, int mods) {};
    virtual void keyEvent(int key, int scancode, int action, int mods) {};
    virtual void resizeEvent(int width, int height) {};
    virtual void draw() {};
    virtual void init() {};
    virtual void deinit() {};

protected:
    static void glfwErrorCallback(int error, const char* description);
    void framebufferSizeCallback(int width, int height);
    GLFWwindow* window = nullptr;
    double deltaTime = 0.0f;
    double lastFrameTime = 0.0f;
    int width = 1280, height = 720;
};

template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)> {
    template <typename... Args>
    static Ret callback(Args... args) {
        func(args...);
    }
    static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;